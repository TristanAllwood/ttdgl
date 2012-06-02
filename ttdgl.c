#define _XOPEN_SOURCE 600 
#define _BSD_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "util.h"

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

static void parent(int pty_master_fd, int pty_child_fd) {
  if (close(pty_child_fd) == -1) {
    die_with_error("close [child]");
  }

  // TODO start up sdl, gl and madness!

  if (SDL_Init(SDL_INIT_VIDEO) == -1) {
    die_with_error("SDL_Init");
  }

  if(atexit(SDL_Quit) != 0) {
    die_with_error("atexit");
  }

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

  screen = SDL_SetVideoMode(640, 480, 0, SDL_HWSURFACE | SDL_OPENGL | SDL_RESIZABLE);

  if (screen == NULL) {
    die_with_error("SDL_SetVideoMode");
  }



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
