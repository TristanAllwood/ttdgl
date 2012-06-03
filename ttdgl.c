#define _XOPEN_SOURCE 600 
#define _BSD_SOURCE

#include <fcntl.h>
#include <unistd.h>

#include "child.h"
#include "parent.h"
#include "render.h"
#include "ttdgl_state.h"
#include "util.h"

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
      parent(fork_result, pty_master_fd, pty_child_fd);
      break;
  }

  return 0;
}
