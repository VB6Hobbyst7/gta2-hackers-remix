#include "sty_parser.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This parser can handle the following chunks (we only want to draw the fonts):
//   FONB, SPRX, SPRG,
// TODO:
//	PALX, PPAL, PALB

// how to access a single letter in theory:
// sprite_id = (character-code - first character) + sty->font_base.base[font_id]
// then look up that ID in the sprite index.

// Font base
void sty_parser_read_FONB(sty_t *sty, char *buffer_pos, uint32_t length) {
  // read and validate the font count
  uint16_t font_count = *(uint16_t *)buffer_pos;
  if (font_count * 2 + 2 != length)
    exit(printf("ERROR: font_count doesn't match chunk size!\n"));

  // read the font base
  uint16_t *base = malloc(2 * font_count);
  for (uint16_t i = 0; i < font_count; i++)
    base[i] = *(uint16_t *)(buffer_pos + 2 + 2 * i) +
              (i ? base[i - 1] : 0); // add the last one, if any

  // save everything in the sty struct
  sty->font_base.font_count = font_count;
  sty->font_base.base = base;
}

// Sprite index
void sty_parser_read_SPRX(sty_t *sty, char *buffer_pos, uint32_t length) {
  // calculate and validate the sprite count
  uint16_t sprite_count = length / sizeof(sprite_entry_t);
  if (length % sizeof(sprite_entry_t))
    exit(printf("ERROR: chunk length isn't a multiple of sprite_entry_t!\n"));

  // read the sprite infos
  sprite_entry_t *sprite_entries =
      malloc(sizeof(sprite_entry_t) * sprite_count);
  for (uint16_t i = 0; i < sprite_count; i++)
    sprite_entries[i] =
        *(sprite_entry_t *)(buffer_pos + sizeof(sprite_entry_t) * i);

  // save everything in the sty struct
  sty->sprite_index.sprite_count = sprite_count;
  sty->sprite_index.sprite_entries = sprite_entries;
}

// Sprite graphics
void sty_parser_read_SPRG(sty_t *sty, char *buffer_pos, uint32_t length) {
  // copy the whole blob into a new buffer
  char *blob = malloc(length);
  for (uint32_t i = 0; i < length; i++)
    blob[i] = buffer_pos[i];

  // save everything in the sty struct
  sty->sprite_blob.blob_length = length;
  sty->sprite_blob.blob = blob;
}

// Palette index
void sty_parser_read_PALX(sty_t *sty, char *buffer_pos, uint32_t length) {
  // TODO
}

// Physical palettes
void sty_parser_read_PPAL(sty_t *sty, char *buffer_pos, uint32_t length) {
  // TODO
}

// Palette Base
void sty_parser_read_PALB(sty_t *sty, char *buffer_pos, uint32_t length) {
  // TODO
}

// returns the next offset or 0
uint32_t sty_parser_read_next_chunk(sty_t *sty, char *buffer, uint32_t offset,
                                    uint32_t sty_size) {
  // read the chunk type and size
  char type[5];
  for (int i = 0; i < 4; i++)
    type[i] = buffer[offset + i];
  type[5] = '\0';
  uint32_t chunk_size = *(uint32_t *)(buffer + offset + 4);

  // check if the chunk size makes sense
  if (!chunk_size || offset >= sty_size)
    exit(printf("ERROR!\n"));

  // read the chunk content
  if (!strcmp("FONB", type))
    sty_parser_read_FONB(sty, buffer + offset + 8, chunk_size);
  if (!strcmp("SPRX", type))
    sty_parser_read_SPRX(sty, buffer + offset + 8, chunk_size);
  if (!strcmp("SPRG", type))
    sty_parser_read_SPRG(sty, buffer + offset + 8, chunk_size);
  if (!strcmp("PALX", type))
    sty_parser_read_PALX(sty, buffer + offset + 8, chunk_size);
  if (!strcmp("PPAL", type))
    sty_parser_read_PPAL(sty, buffer + offset + 8, chunk_size);
  if (!strcmp("PALB", type))
    sty_parser_read_PALB(sty, buffer + offset + 8, chunk_size);

  // return the new offset (or 0 if we're done here)
  offset += chunk_size + 8 /* chunk header */;
  if (offset == sty_size)
    return 0;
  return offset;
}

sty_t *sty_parser(char *filename) {
  printf("loading %s...\n", filename);

  // open the file
  FILE *handle;
  handle = fopen(filename, "rb");
  if (handle == NULL)
    exit(printf("Can't open the file!\n"));

  // calculate file size
  int size;
  fseek(handle, 0, SEEK_END);
  size = ftell(handle);

  // read the whole file into RAM (<<10 MB) and close it
  char *buffer;
  rewind(handle);
  buffer = (char *)malloc(sizeof(char) * size);
  if (fread(buffer, 1, size, handle) != size)
    exit(printf("Read error!\n"));
  fclose(handle);

  // check the file header
  if (buffer[0] != 'G' || buffer[1] != 'B' || buffer[2] != 'S' ||
      buffer[3] != 'T')
    exit(printf("This isn't a GBST file!\n"));

  // create an empty sty structure
  sty_t *sty = malloc(sizeof(sty_t));
  sty->font_base.font_count = 0;
  sty->sprite_index.sprite_count = 0;
  sty->sprite_blob.blob_length = 0;

  // fill the sty
  uint32_t offset = 6; // skip the header!
  while (offset)
    offset = sty_parser_read_next_chunk(sty, buffer, offset, size);

  free(buffer);
  return sty;
}

void sty_cleanup(sty_t *sty) {
  if (sty->font_base.font_count)
    free(sty->font_base.base);
  if (sty->sprite_index.sprite_count)
    free(sty->sprite_index.sprite_entries);
  if (sty->sprite_blob.blob_length)
    free(sty->sprite_blob.blob);
  free(sty);
}
