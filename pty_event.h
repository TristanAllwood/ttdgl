#ifndef VTY_EVENT_H
#define VTY_EVENT_H

enum pty_event_code {
  PTY_CLOSED, PTY_WRITE, PTY_SET_ATTRIBUTE
};

enum { BUFFER_SIZE = 2048 };

typedef struct pty_write {
  char nt_unicode_char[5];
} pty_write_t;

typedef struct pty_set_attributes {
  long int attr_codes[16];
  int arg_count;
} pty_set_attributes_t;

#endif
