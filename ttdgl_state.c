#include "ttdgl_state.h"
#include "util.h"

const int DEFAULT_WIDTH = 640;
const int DEFAULT_HEIGHT = 480;

ttdgl_state_t * init_ttdgl_state(void) { 
  
  ttdgl_state_t * state = malloc(sizeof(ttdgl_state_t));

  surface_resize(DEFAULT_WIDTH, DEFAULT_HEIGHT, state);

  return state;
}

void surface_resize(int width, int height, ttdgl_state_t * state) {
  SDL_Surface * surface = SDL_SetVideoMode(width, height, 0, SDL_HWSURFACE | SDL_OPENGL | SDL_RESIZABLE);
  if (surface == NULL) {
    die_with_error("SDL_SetVideoMode");
  }

  state->surface_width = width;
  state->surface_height = height;
  state->surface = surface;
}
