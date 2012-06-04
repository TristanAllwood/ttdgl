#include <GL/gl.h>
#include <GL/glx.h>

#include "render.h"

static const GLfloat FONT_SIZE = 12.00f;

static const GLfloat FAR_DEPTH = 1.0f;
static const GLfloat BOLD_DEPTH = 0.001f;
static const GLfloat EYE_GAP = 0.01f;

static void render_scene(GLfloat xoff, ttdgl_state_t * state);


void render(ttdgl_state_t * state) {

  glClearColor(1, 1, 1, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glLoadIdentity();

  glViewport(0, 0, state->surface_width / 2, state->surface_height);
  render_scene(-EYE_GAP, state);

  glLoadIdentity();

  glViewport(state->surface_width / 2, 0, state->surface_width / 2, state->surface_height);
  render_scene(EYE_GAP, state);

  SDL_GL_SwapBuffers();

}


static void render_scene(GLfloat xoff, ttdgl_state_t * state) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  //glOrtho(0, state->surface_width, 0, state->surface_height, 1, -1);
  glFrustum(0, (state->surface_width / 2), 0, state->surface_height, FAR_DEPTH, FAR_DEPTH + 100); 

  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_TEXTURE_2D);
  glLoadIdentity();

  glColor3f(0, 1, 0);

  glTranslatef(-xoff, 0,0);

  ftglSetFontFaceSize(state->font, FONT_SIZE, FONT_SIZE);

  cell_t * cell = state->current_frame->cells;
  frame_t * current_frame = state->current_frame;
  uint num_rows = current_frame->rows;
  uint num_cols = current_frame->cols;

  GLfloat act_width = (FONT_SIZE * num_cols) / 2.0f;
  GLfloat act_height = FONT_SIZE * num_rows;

  GLfloat gap_x = ((state->surface_width / 2.0f) - act_width) / 2.0f;
  GLfloat gap_y = (state->surface_height - act_height) / 2.0f;

  glTranslatef(gap_x, (FONT_SIZE * num_rows) + gap_y, -(FAR_DEPTH + 0.001f));

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
        glTranslatef(0,0,-BOLD_DEPTH);
      }

      ftglRenderFont(state->font, chars, FTGL_RENDER_ALL); 
      if ((cell->attrs.attr_flags & ATTR_BOLD) == ATTR_BOLD) {
        glTranslatef(0,0,BOLD_DEPTH);
      }

      glTranslatef(FONT_SIZE / 2,0,0);
    }
    glTranslatef(-FONT_SIZE * ((int) num_cols) / 2, -FONT_SIZE, 0);
  }
}
