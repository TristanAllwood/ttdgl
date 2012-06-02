#ifndef TTDGL_STATE_H
#define TTDGL_STATE_H

#include <SDL.h>
#include <stdlib.h>

typedef struct ttdgl_state {
  int pty_master_fd;
  int surface_width;
  int surface_height;
  SDL_Surface * surface;
} ttdgl_state_t;

ttdgl_state_t * init_ttdgl_state(int pty_master_fd);

void surface_resize(int width, int height, ttdgl_state_t * state);

#endif
