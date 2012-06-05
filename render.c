#include <GL/gl.h>
#include <GL/glx.h>

#include "render.h"

static void render_scene(GLfloat xoff, ttdgl_state_t * state);


void render(ttdgl_state_t * state) {

  GLfloat eye_gap = state->render_state.eye_gap;

  glClearColor(1, 1, 1, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glLoadIdentity();

  glViewport(0, 0, state->surface_width / 2, state->surface_height);
  render_scene(-eye_gap, state);

  glLoadIdentity();

  glViewport(state->surface_width / 2, 0, state->surface_width / 2, state->surface_height);
  render_scene(eye_gap, state);

  SDL_GL_SwapBuffers();

}


static void render_scene(GLfloat xoff, ttdgl_state_t * state) {

  GLfloat font_size = state->render_state.font_size;
  GLfloat far_depth = state->render_state.far_depth;
  GLfloat bold_depth = state->render_state.bold_depth;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  //glOrtho(0, state->surface_width, 0, state->surface_height, 1, -1);
  glFrustum(0, (state->surface_width / 2), 0, state->surface_height, far_depth, far_depth + 100); 

  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_TEXTURE_2D);
  glLoadIdentity();

  glColor3f(0, 1, 0);

  glTranslatef(xoff * 100, 0,0);

  ftglSetFontFaceSize(state->font, font_size, font_size);

  cell_t * cell = state->current_frame->cells;
  frame_t * current_frame = state->current_frame;
  uint num_rows = current_frame->rows;
  uint num_cols = current_frame->cols;

  GLfloat act_width = (font_size * num_cols) / 2.0f;
  GLfloat act_height = font_size * num_rows;

  GLfloat gap_x = ((state->surface_width / 2.0f) - act_width) / 2.0f;
  GLfloat gap_y = (state->surface_height - act_height) / 2.0f;

  glTranslatef(gap_x, (font_size * num_rows) + gap_y, -(far_depth + 0.001f));

  for (int i = 0 ; i < num_rows ; ++i) {
    for (int j = 0 ; j < num_cols ; ++j, ++cell) {
      char * chars = cell->nt_unicode_char;

      uint32_t color = cell->attrs.foreground_colour;
      GLubyte red, green, blue;
      red   = (color & 0xff0000) >> 2;
      green = (color & 0x00ff00) >> 1;
      blue  = (color & 0x0000ff) >> 0;
      glColor3ub(red,green,blue);

      if ((cell->attrs.attr_flags & ATTR_BOLD) == ATTR_BOLD) {
        glTranslatef(0,0,-bold_depth);
      }

      ftglRenderFont(state->font, chars, FTGL_RENDER_ALL); 
      if ((cell->attrs.attr_flags & ATTR_BOLD) == ATTR_BOLD) {
        glTranslatef(0,0,bold_depth);
      }

      glTranslatef(font_size / 2,0,0);
    }
    glTranslatef(-font_size * ((int) num_cols) / 2, -font_size, 0);
  }
}
