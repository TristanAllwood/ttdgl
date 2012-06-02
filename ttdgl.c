#define _XOPEN_SOURCE 600 

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main(int argc, char ** argv) {

  int pty_master_fd = posix_openpt(O_RDWR);

  if (pty_master_fd == -1) {
    int error = errno;
    char * error_message = strerror(error);
    fprintf(stderr, "Internal error: %s\n", error_message);
  }

  return 0;
}
