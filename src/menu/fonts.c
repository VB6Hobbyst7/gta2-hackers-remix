#include "menu.h"
#include <stdint.h>

// SDL2 pixel drawing:
// http://www.programmersranch.com/2014/02/sdl2-pixel-drawing.html

// This isn't working properly yet, just drawing some ascii output currently

// font 9, letter 1 (-> sprite 1147, w: 12, h:18) looks like the number '3'

// TODO: create an enum for font IDs
// TODO: check lengths!
void letter_draw(menu_t *menu, char font_id, uint16_t left, uint16_t top,
                 char letter) {
  // display all font base numbers
  for (int i = 0; i < menu->fstyle->font_base.font_count; i++) {
    printf("%i: %i\n", i, menu->fstyle->font_base.base[i]);
  }

  font_id = 9;
  letter = 'A';

  /*for(font_id=3;font_id<13;font_id++)*/ for (int letter_id = 0;
                                               letter_id < 20; letter_id++) {
    printf("font_id: %i, letter_id: %i\n", font_id, letter_id);

    // get the sprite information
    int sprite_id = letter_id + menu->fstyle->font_base.base[font_id];
    sprite_entry_t sprite_entry =
        menu->fstyle->sprite_index.sprite_entries[sprite_id];
    int width = sprite_entry.width;
    int height = sprite_entry.height;
    printf("sprite_id: %i, w: %i, h: %i\n", sprite_id, width, height);

    // max. allowed size: 128x64
    if (width > 128 || height > 64)
      exit(printf("Sprite %i has invalid width and/or height.\n", sprite_id));

    // save the sprite in a pixel array
    char *sprite = menu->fstyle->sprite_blob.blob + sprite_entry.ptr;

    // output the sprite - debug!
    for (int i = -1; i < height + 1; i++) {
      for (int j = -1; j < width + 1; j++) {
        if (i == -1 || i == height)
          printf("-");
        else {
          if (j == -1 || j == width)
            printf("|");
          else {
            char color = sprite[(i * width) + j];
            if (color) {
              // printf("%02x ",  color & 0xff);
              printf("X");
            } else
              printf(" ", color);
          }
        }
      }
      printf("\n");
    }
  }

  /*
  uint32_t* pixels = malloc(sizeof(uint32_t) * width * height);


  // create a new texture
  SDL_Texture * tex = SDL_CreateTexture(menu->renderer,
          SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC,
          width, height);

  SDL_UpdateTexture(tex, NULL, pixels, width * sizeof(uint32_t));
  free(pixels);
  */
}
