#include "ttdgl_state.h"
#include "util.h"

static const int DEFAULT_WIDTH = 640;
static const int DEFAULT_HEIGHT = 480;

ttdgl_state_t * init_ttdgl_state(pid_t child_pid, int pty_master_fd) { 
  
  ttdgl_state_t * state = malloc(sizeof(ttdgl_state_t));

  state->child_pid = child_pid;
  state->pty_master_fd = pty_master_fd;
  surface_resize(DEFAULT_WIDTH, DEFAULT_HEIGHT, state);
  
  cursor_t cursor;
  cursor.x = 0;
  cursor.y = 0;

  state->current_cursor = cursor;
  state->saved_cursor   = cursor;

  attrs_t attrs;
  attrs.line_wrap = true;
  attrs.alt_font  = false;

  state->current_attrs = attrs;
  state->saved_attrs = attrs;

  frame_t * initial_frame = malloc(sizeof(frame_t));
  initial_frame->rows = 24;
  initial_frame->cols = 80;

  cell_t ** cells = calloc(initial_frame->rows, sizeof(cell_t *));
  for(int i = 0 ; i < initial_frame->rows ; i++) {
    cells[i] = calloc(initial_frame->cols, sizeof(cell_t));

    for(int j = 0 ; j < initial_frame->cols; j++) {
      cells[i][j].nt_unicode_char[0] = '~';
      cells[i][j].alt_font = false;
      cells[i][j].foreground_colour=0xFFFFFF00;
      cells[i][j].background_colour=0x00000000;
    }
  }

  state->current_frame=initial_frame;


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
