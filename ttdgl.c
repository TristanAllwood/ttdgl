#define _XOPEN_SOURCE 600 
#define _BSD_SOURCE

#include <stdbool.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>
#include <SDL_thread.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "util.h"

static SDL_Surface * screen_init_resize(int width, int height);

void render(void) {
}

static void handle_sdl_quit(void) {
  exit(0);
}

static void handle_sdl_resize(SDL_ResizeEvent * event, SDL_Surface ** screen) {
  (*screen) = screen_init_resize(event->w, event->h);
}

static void main_loop(SDL_Surface * screen) {
  while(true) {
    render();

    SDL_Event event;

    if (SDL_WaitEvent(&event) == 0) {
      die_with_error("SDL_WaitEvent");
    }

    do {
      switch (event.type) {
        case SDL_QUIT: 
          handle_sdl_quit();
          exit(0);
        case SDL_VIDEORESIZE:
          handle_sdl_resize(&event.resize, &screen);
          break;

        default:
          fprintf(stderr, "Unknown event: %x\n", event.type);
          break;
      }
    } while (SDL_PollEvent(&event));
  }
}

static const char * get_shell() {
  char * shell = NULL;
  shell = getenv("SHELL"); 

  if (shell != NULL) {
    return shell;
  }

  errno = 0;
  struct passwd * pw_ent = getpwent();
  if (pw_ent == NULL && errno != 0) {
    die_with_error("getpwent");
  }

  if (pw_ent != NULL && pw_ent->pw_shell != NULL) {
    return pw_ent->pw_shell;
  }

  return "/bin/sh";
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

  SDL_Surface * screen;
  
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

  screen = screen_init_resize(640, 480);
  main_loop(screen);

}



static SDL_Surface * screen_init_resize(int width, int height) {
  SDL_Surface * screen = SDL_SetVideoMode(width, height, 0, SDL_HWSURFACE | SDL_OPENGL | SDL_RESIZABLE);
  if (screen == NULL) {
    die_with_error("SDL_SetVideoMode");
  }
  return screen;
}

static void child(int pty_master_fd, int pty_child_fd) {
  
  if (close(pty_master_fd) == -1) {
    die_with_error("close[master]");
  }

  struct termios term_settings;

  if (tcgetattr(pty_child_fd, &term_settings) == -1) {
    die_with_error("tcgetattr");
  }

  cfmakeraw(&term_settings);

  if(tcsetattr(pty_child_fd, TCSANOW, &term_settings) == -1) {
    die_with_error("tcsetattr");
  }

  if (dup2(pty_child_fd, 0) == -1) {
    die_with_error("dup2 [stdin]");
  }

  if (dup2(pty_child_fd, 1) == -1) {
    die_with_error("dup2 [stdout]");
  }

  if (dup2(pty_child_fd, 2) == -1) {
    die_with_error("dup2 [stderr]");
  }

  if (close(pty_child_fd) == -1) {
    die_with_error("close[child]");
  }

  if (setsid() == -1) {
    die_with_error("setsid");
  }

  if (ioctl(0, TIOCSCTTY, 1) == -1) {
    die_with_error("ioctl");
  }

  const char * shell = get_shell();
  char * const * args = calloc(0, sizeof(char *));
  if (execv(shell, args) == -1) {
    die_with_error("execv");
  }
  
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
