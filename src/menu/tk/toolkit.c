#include "toolkit.h"
#include "actions.h"

tk_t *tk_init(gxt_t *gxt, sty_t *fsty, sfx_t *sfx, bg_t *bg, char *title) {
  tk_t *tk = calloc(1, sizeof(tk_t));
  tk->sfx = sfx;
  tk->fsty = fsty;
  tk->bg = bg;
  tk->gxt = gxt;

  // create the SDL window
  tk->window =
      SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                       640, 480, SDL_WINDOW_SHOWN);
  if (!tk->window)
    exit(printf("SDL_ERROR: %s\n", SDL_GetError()));

  tk->renderer = SDL_CreateRenderer(tk->window, -1, 0);

  // set the virtual resolution to 640x480
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize(tk->renderer, 640, 480);

  return tk;
}

// event is NULL when if the timelimit for the next
// frame has been reached
void tk_frame(tk_t *tk, SDL_Event *event) {
  tk_action(tk, event);
  if (tk->redraw_needed) {
    tk->redraw_needed = 0;
    SDL_RenderClear(tk->renderer);
    tk_screen_draw(tk);
    SDL_RenderPresent(tk->renderer);
  }
}

void tk_cleanup(tk_t *tk) {
  tk_el_clean(tk, &(tk->screen_first->el));

  SDL_DestroyRenderer(tk->renderer);
  SDL_DestroyWindow(tk->window);
  free(tk);
}
