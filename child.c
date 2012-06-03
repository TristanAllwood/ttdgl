#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>

#include "util.h"
#include "child.h"

static char * get_shell();

void child(int pty_master_fd, int pty_child_fd) {
  
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

  if (close(0) == -1) {
    die_with_error("close[stdin]");
  }
  if (close(1) == -1) {
    die_with_error("close[stdout]");
  }
  if (close(2) == -1) {
    die_with_error("close[stderr] - ?");
  }

  if (dup2(pty_child_fd, 0) == -1) {
    die_with_error("dup2 [stdin]");
  }

  if (dup2(pty_child_fd, 1) == -1) {
    die_with_error("dup2 [stdout]");
  }

  if (dup2(pty_child_fd, 2) == -1) {
    die_with_error("dup2 [stderr] - ?");
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

  char * shell = get_shell();
  char * argv[2];
  argv[0] = shell;
  argv[1] = NULL;

  if (execv("/usr/bin/luit", argv) == -1) {
    die_with_error("execv");
  }
  
}

static char * get_shell() {
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
