#define _XOPEN_SOURCE 600 
#define _BSD_SOURCE

#include <stdbool.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>
#include <SDL_thread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"
#include "ttdgl_state.h"
#include "child.h"
#include "render.h"

static void handle_sdl_quit(void) {
  exit(0);
}

static void handle_sdl_resize(SDL_ResizeEvent * event, ttdgl_state_t * state) {
  surface_resize(event->w, event->h, state);
}

static const uint16_t MAX_EVENTS_BEFORE_RENDER = 1000;

static void main_loop(ttdgl_state_t * state) {
  while(true) {
    render(state);

    uint16_t event_count = 0;

    SDL_Event event;

    if (SDL_WaitEvent(&event) == 0) {
      die_with_error("SDL_WaitEvent");
    }

    do {
      switch (event.type) {
        case SDL_QUIT: 
          handle_sdl_quit();
          break;
        case SDL_VIDEORESIZE:
          handle_sdl_resize(&event.resize, state);
          break;

        default:
          fprintf(stderr, "Unknown event: %x\n", event.type);
          break;
      }
    } while (++event_count < MAX_EVENTS_BEFORE_RENDER &&
              SDL_PollEvent(&event));
  }
}


static int parent_pty_event_loop(void * data) {
  int pty_master_fd = (int) data;

  // TODO: epoll loop that pushes to the SDL event queue.
  return pty_master_fd;
  
}

static void parent(int pty_master_fd, int pty_child_fd) {
  if (close(pty_child_fd) == -1) {
    die_with_error("close [child]");
  }

  if (SDL_Init(SDL_INIT_VIDEO) == -1) {
    die_with_error("SDL_Init");
  }

  if(atexit(SDL_Quit) != 0) {
    die_with_error("atexit");
  }

  SDL_CreateThread(parent_pty_event_loop, (void *) pty_master_fd);

  if (SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 ) == -1) {
    die_with_error("SDL_GL_SetAttribute[RED]");
  }

  if (SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 )) {
    die_with_error("SDL_GL_SetAttribute[GREEN]");
  }
  if (SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 )) {
    die_with_error("SDL_GL_SetAttribute[BLUE]");
  }
  if (SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 )) {
    die_with_error("SDL_GL_SetAttribute[DEPTH]");
  }
  if (SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 )) {
    die_with_error("SDL_GL_SetAttribute[DOUBLEBUFFER]");
  }

  ttdgl_state_t * state = init_ttdgl_state();

  main_loop(state);

}


int main(int argc, char ** argv) {

  int pty_master_fd = posix_openpt(O_RDWR);
  if (pty_master_fd == -1) {
    die_with_error("posix_openpt");
  }

  if (grantpt(pty_master_fd) == -1) {
    die_with_error("grantpt");
  }

  if (unlockpt(pty_master_fd) == -1) {
    die_with_error("unlockpt");
  }

  char * pty_child = ptsname(pty_master_fd);
  if (pty_child == NULL) {
    die_with_error("ptsname");
  }

  int pty_child_fd = open(pty_child, O_RDWR);
  if (pty_child_fd == -1) {
    die_with_error("open[pty_child]");
  }

  pid_t fork_result = fork();

  switch (fork_result) {
    case -1:
      die_with_error("fork");
    case 0:
      child(pty_master_fd, pty_child_fd); 
      break;
    default:
      parent(pty_master_fd, pty_child_fd);
      break;
  }

  return 0;
}
