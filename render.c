#include <GL/gl.h>
#include <GL/glx.h>

#include "render.h"

static const int FONT_SIZE = 20;

void render(ttdgl_state_t * state) {


  glClearColor(1, 1, 1, 0);

  glViewport(0, 0, state->surface_width, state->surface_height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(0, state->surface_width, 0, state->surface_height, 1, -1);

  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_TEXTURE_2D);
  glLoadIdentity();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  glColor3f(0, 1, 0);

  ftglSetFontFaceSize(state->font, FONT_SIZE, FONT_SIZE);

  cell_t * cell = state->current_frame->cells;
  frame_t * current_frame = state->current_frame;
  uint num_rows = current_frame->rows;
  uint num_cols = current_frame->cols;

  glTranslatef(0, FONT_SIZE * num_rows,0);

  for (int i = 0 ; i < num_rows ; ++i) {
    for (int j = 0 ; j < num_cols ; ++j, ++cell) {
      char * chars = cell->nt_unicode_char;
      ftglRenderFont(state->font, chars, FTGL_RENDER_ALL);
      glTranslatef(FONT_SIZE / 2,0,0);
    }
    glColor3f(1, 0, 0);
    glTranslatef(-FONT_SIZE * ((int) num_cols) / 2, -FONT_SIZE, 0);
  }

  SDL_GL_SwapBuffers();

}
