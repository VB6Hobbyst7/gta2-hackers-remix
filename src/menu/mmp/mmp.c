#include "mmp.h"
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// little helping function that removes whitespaces around a string,
// for example: ' hello world!  ' becomes 'hello world!'
char *trim(char *str, int from) {
  // find whitespace positions, that we'll rip off the string
  int len = strlen(str);
  int trimmed_start = 0;
  int trimmed_end = len;
  char non_space_found = 0;
  for (int i = from; i < len; i++) {
    if (isspace(str[i])) {
      if (!non_space_found && (!trimmed_start || trimmed_start == i))
        trimmed_start = i + 1;
      else if (non_space_found && trimmed_end == len)
        trimmed_end = i;
    } else {
      non_space_found = 1;
      trimmed_end = len;
    }
  }

  // now move the chars to the left and cut the string off
  int offset_start = trimmed_start - from;
  if (offset_start < 0)
    offset_start = 0;

  for (int i = from; i < len; i++) {
    if (i + offset_start == trimmed_end) {
      str[i] = '\0';
      break;
    }
    str[i] = str[i + offset_start];
  }

  // trading a few bytes for additional CPU time. For G2HR this won't
  // really matter anyway, except when you have a gazillion custom
  // maps in your data folder.
  return realloc(str, trimmed_end - offset_start + 1);
}

mmp_key_t *mmp_parse(char *buffer, int size) {
  mmp_key_t *first = NULL;
  mmp_key_t *last = NULL;

  char ignore_line = 0;

  int start_brackets = -1;
  int start_key = -1;
  int start_line = 0;

  char *section = NULL;
  int section_length = 0;
  for (int i = 0; i <= size; i++) {
    char c = i < size ? buffer[i] : '\n'; // add an extra line end
    if (c == '[')
      start_brackets = i + 1;
    else if (c == ';' && i == start_line)
      ignore_line = 1;
    else if (c == ']') {
      if (start_brackets == -1)
        exit(printf("ERROR: unexpected ']' without '[' in the same line!"));

      if (section)
        free(section);
      section_length = i - start_brackets;
      section = malloc(section_length + 1);
      strncpy(section, buffer + start_brackets, section_length);
      section[section_length] = '\0';
    } else if (c == '\n' || c == '\r') {
      if (start_key != -1 && !ignore_line) {
        int key_length = start_key - start_line - 1;
        int key_section_length = section_length + 1 /* slash */
                                 + key_length;
        int value_length = i - start_key;

        mmp_key_t *new = malloc(sizeof(mmp_key_t));
        new->next = NULL;
        new->key = malloc(key_section_length + 1);
        new->value = malloc(value_length + 1);

        // create 'key' string
        strncpy(new->key, section, section_length);
        strncpy(new->key + section_length + 1, buffer + start_line, key_length);
        new->key[section_length] = '/';
        new->key[key_section_length] = '\0';
        new->key = trim(new->key, section_length + 1);

        // create 'value' string
        strncpy(new->value, buffer + start_key, value_length);
        new->value[value_length] = '\0';
        new->value = trim(new->value, 0);

        // add it to the list
        if (last)
          last->next = new;
        else
          first = new;
        last = new;
      }
      ignore_line = 0;
      start_brackets = -1;
      start_key = -1;
      start_line = i + 1;
    } else if (c == '=') {
      start_key = i + 1;
    }
  }

  if (section)
    free(section);
  return first;
}

mmp_t *mmp_load(char *filename) {
  mmp_t *mmp = malloc(sizeof(mmp_t));
  mmp->source = filename;
  mmp->next = NULL;

  FILE *handle = fopen(filename, "rb");
  if (!handle)
    exit(printf("ERROR: Couldn't read '%s'!", filename));

  // calculate file size
  int size;
  fseek(handle, 0, SEEK_END);
  size = ftell(handle);

  // read the whole file into RAM (<<10 MB) and close it
  char *buffer;
  rewind(handle);
  buffer = (char *)malloc(size);
  if (fread(buffer, 1, size, handle) != size)
    exit(printf("Read error while reading '%s'!", filename));
  fclose(handle);

  mmp->data = mmp_parse(buffer, size);
  free(buffer);
  return mmp;
}

// load all files in the data/ folder
mmp_t *mmp_init(const char *path) {
  printf("loading %s/*.mmp...\n", path);

  DIR *dir = opendir(path);
  if (!dir)
    exit(printf("Couldn't find path '%s'!\n", path));

  mmp_t *first = NULL;
  mmp_t *last = NULL;
  size_t len_path = strlen(path);
  while (1) {
    struct dirent *entry = readdir(dir);
    if (!entry)
      break;

    // skip non-.mmp-files
    char *name = entry->d_name;
    size_t len = strlen(name);
    if (len < 4 || name[len - 4] != '.' ||
        (name[len - 3] != 'm' && name[len - 3] != 'M') ||
        (name[len - 2] != 'm' && name[len - 2] != 'M') ||
        (name[len - 1] != 'p' && name[len - 1] != 'P'))
      continue;

    // load the file
    char *fullpath = malloc(len_path + len + 2);
    snprintf(fullpath, len_path + len + 2, "%s/%s", path, name);
    mmp_t *new = mmp_load(fullpath);

    // add it to the list
    if (first)
      last->next = new;
    else
      first = new;
    last = new;
  }
  closedir(dir);

  return first;
}

char *mmp_read(mmp_t *mmp, char *key) {
  mmp_key_t *current = mmp->data;
  while (current) {
    if (!strcmp(current->key, key))
      return current->value;
    current = current->next;
  }
  return "";
}

void mmp_cleanup(mmp_t *mmp) {
  while (mmp) {
    // free all keys
    mmp_key_t *data = mmp->data;
    while (data) {
      mmp_key_t *old = data;
      data = data->next;
      free(old->key);
      free(old->value);
      free(old);
    }

    // free the mmp struct itself
    mmp_t *old = mmp;
    mmp = mmp->next;
    free(old->source);
    free(old);
  }
}
