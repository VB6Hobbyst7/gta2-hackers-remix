#include "cstr.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  char *trimmed = cstr_trim(cstr_copy("       trim me     "));
  printf("COPY & TRIM:\n");
  printf("\t\"%s\"\n", trimmed);
  free(trimmed);

  printf("MERGE:\n");
  char *merged = cstr_merge("hello", " world!             ", "look", "a",
                            "merged", "string! :)");

  printf("\t%s\n", merged);

  printf("SPLIT (#1: by space character):\n");
  cstr_split_t *split = cstr_split(merged, ' ', false);
  for (int i = 0; i < split->count; i++)
    printf("\tpiece %i: %s\n", i, split->pieces[i]);
  cstr_split_free(split);
  free(merged);

  printf("SPLIT (#2: by comma + trim):\n");
  char *numbers = "1, 2, 3, 4, 5";
  printf("\tsplitting this string: %s\n", numbers);
  split = cstr_split(numbers, ',', true);
  for (int i = 0; i < split->count; i++)
    printf("\tpiece %i: \"%s\"\n", i, split->pieces[i]);
  cstr_split_free(split);
}
