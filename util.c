#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void die_with_error(const char * method) {
    int error = errno;
    char * error_message = strerror(error);
    fprintf(stderr, "error with %s: %s\n", method, error_message);
    exit(error);
}
