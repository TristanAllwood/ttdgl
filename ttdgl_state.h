#ifndef TTDGL_STATE_H
#define TTDGL_STATE_H

#include <FTGL/ftgl.h>
#include <GL/gl.h>
#include <SDL.h>
#include <stdbool.h>
#include <stdlib.h>

enum attr_flag {
  ATTR_BRIGHT     = 1 << 0,
  ATTR_DIM        = 1 << 1,
  ATTR_UNDERSCORE = 1 << 2,
  ATTR_BLINK      = 1 << 3,
  ATTR_REVERSE    = 1 << 4,
  ATTR_HIDDEN     = 1 << 5,
  ATTR_BOLD       = 1 << 6
};

typedef struct attrs {

  int attr_flags;
  bool line_wrap;
  bool alt_font;
  uint32_t foreground_colour;
  uint32_t background_colour; 

} attrs_t;

typedef struct cell {
  char nt_unicode_char[5];
  attrs_t attrs;
} cell_t;

typedef struct frame {
  uint rows;
  uint cols;

  cell_t * cells;

  struct frame * previous_frame;
} frame_t;

typedef struct cursor {
  uint x;
  uint y;
} cursor_t;

typedef struct render_state {
  GLfloat font_size;
  GLfloat far_depth;
  GLfloat bold_depth;
  GLfloat eye_gap;
} render_state_t;

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

  FTGLfont * font;
  FTGLfont * alt_font;

  frame_t * current_frame;

  render_state_t render_state;
  
} ttdgl_state_t;


ttdgl_state_t * init_ttdgl_state(pid_t child_pid, int pty_master_fd);

void surface_resize(int width, int height, ttdgl_state_t * state);
void put_char(char nt_unicode_char[5], ttdgl_state_t * state);

#endif
