#ifndef TTDGL_STATE_H
#define TTDGL_STATE_H

#include <stdbool.h>
#include <SDL.h>
#include <stdlib.h>

enum attr_flag {
  ATTR_BRIGHT     = 1 << 0,
  ATTR_DIM        = 1 << 1,
  ATTR_UNDERSCORE = 1 << 2,
  ATTR_BLINK      = 1 << 3,
  ATTR_REVERSE    = 1 << 4,
  ATTR_HIDDEN     = 1 << 5
};

typedef struct cell {
  char nt_unicode_char[5];

  bool alt_font;
  int attr_flags;

  uint32_t foreground_colour;
  uint32_t background_colour; 
} cell_t;

typedef struct frame {
  uint rows;
  uint cols;

  cell_t ** cells;

  struct frame * previous_frame;
} frame_t;

typedef struct cursor {
  uint x;
  uint y;
} cursor_t;


typedef struct attrs {

  bool line_wrap;
  bool alt_font;

} attrs_t;

typedef struct ttdgl_state {
  pid_t child_pid;
  int pty_master_fd;
  int surface_width;
  int surface_height;
  SDL_Surface * surface;

  cursor_t current_cursor;
  cursor_t saved_cursor;

  attrs_t current_attrs;
  attrs_t saved_attrs;

  frame_t * current_frame;
} ttdgl_state_t;

ttdgl_state_t * init_ttdgl_state(pid_t child_pid, int pty_master_fd);

void surface_resize(int width, int height, ttdgl_state_t * state);

#endif
