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

  state->font = ftglCreateTextureFont("/usr/share/fonts/truetype/freefont/FreeMono.ttf");
  if (state->font == NULL) {
    die_with_error("ftglCreateTextureFont");
  }

  state->alt_font = ftglCreateTextureFont("/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono.ttf");
  if (state->alt_font == NULL) {
    die_with_error("ftglCreateTextureFont[alt]");
  }

  frame_t * initial_frame = malloc(sizeof(frame_t));
  initial_frame->rows = 24;
  initial_frame->cols = 80;

  cell_t * cells = calloc(initial_frame->rows * initial_frame->cols, sizeof(cell_t));

  for (cell_t * cell = cells ; 
      cell < cells + (initial_frame->rows * initial_frame->cols) ; 
      ++cell) {
    cell->nt_unicode_char[0] = '~';
    cell->nt_unicode_char[1] = '\0';
    cell->alt_font = false;
    cell->foreground_colour=0xFFFFFF00;
    cell->background_colour=0x00000000;
  }
  initial_frame->cells = cells;

  state->current_frame = initial_frame;


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


void put_char(char nt_unicode_char[5], ttdgl_state_t * state) {

  frame_t * frame = state->current_frame;
  cursor_t cursor = state->current_cursor; 

  int nRows = frame->rows;
  int nCols = frame->cols;

  cell_t * cells = frame->cells;
  cell_t * cell = cells + (nCols * cursor.y) + cursor.x;

  switch (nt_unicode_char[0]) {
    case '\n':
    case '\r':
      cursor.y++;
      cursor.x = 0;
      break;
    default:
      memcpy(cell->nt_unicode_char, nt_unicode_char, 5);
      cursor.x++;

      if (cursor.x >= nCols) {
        cursor.x = 0;
        cursor.y++;
      } 

  }

  if (cursor.y >= nRows) {
    // TODO: horrible hack if we fall off the bottom.
    cursor.x = 0;
    cursor.y = 0;
  }
  state->current_cursor = cursor;
}
