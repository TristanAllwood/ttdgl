#include <GL/gl.h>
#include <GL/glx.h>

#include "render.h"

void render(ttdgl_state_t * state) {
  cell_t * cell = state->current_frame->cells;

  char * chars = cell->nt_unicode_char;

  glClearColor(1, 1, 1, 0);

  glViewport(0, 0, 640, 480);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(0, 640, 480, 0, 1, -1);

  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_TEXTURE_2D);
  glLoadIdentity();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  glColor3f(0, 1, 0);
  ftglSetFontFaceSize(state->font, 72, 72);
  ftglRenderFont(state->font, chars, FTGL_RENDER_ALL);

  SDL_GL_SwapBuffers();

}
