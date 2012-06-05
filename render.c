#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "render.h"

static void render_scene(GLfloat xoff, ttdgl_state_t * state);


void render(ttdgl_state_t * state) {


  glClearColor(1, 1, 1, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glLoadIdentity();

  glViewport(0, 0, state->surface_width / 2, state->surface_height);
  render_scene(-1, state);

  glLoadIdentity();

  glViewport(state->surface_width / 2, 0, state->surface_width / 2, state->surface_height);
  render_scene(1, state);

  SDL_GL_SwapBuffers();

}


static void render_scene(GLfloat mult, ttdgl_state_t * state) {

  GLfloat font_size = state->render_state.font_size;
  GLfloat bold_depth = state->render_state.bold_depth;

  frame_t * current_frame = state->current_frame;
  uint num_rows = current_frame->rows;
  uint num_cols = current_frame->cols;


  GLfloat act_width = (font_size * num_cols) / 2.0f;
  GLfloat act_height = font_size * num_rows;

  GLfloat aspect_ratio = (state->surface_width / (double) state->surface_height) / 2;

  GLfloat eye_gap   = 20;
  GLfloat fo        = 7;
  GLfloat near_dist = 7;
  GLfloat far_dist  = 20;


  GLfloat width_div_2 = act_width / 2;

  GLfloat top     = width_div_2;
  GLfloat bottom  = -width_div_2;
  GLfloat left    = - aspect_ratio * width_div_2 + mult * 0.5 * eye_gap * near_dist / fo;
  GLfloat right   =   aspect_ratio * width_div_2 + mult * 0.5 * eye_gap * near_dist / fo;



  GLfloat center_x = act_width / 2;
  GLfloat center_y = act_height / 2;


  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glFrustum(left, right, bottom, top, near_dist, far_dist); 

  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_TEXTURE_2D);
  glLoadIdentity();

  glColor3f(0, 1, 0);

  gluLookAt(center_x - mult * eye_gap, center_y, 0,
            center_x - mult * eye_gap, center_y, -1,
            0, 1, 0);

  ftglSetFontFaceSize(state->font, font_size, font_size);

  cell_t * cell = state->current_frame->cells;
  glTranslatef(0, font_size * num_rows, -10);

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
