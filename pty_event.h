#ifndef VTY_EVENT_H
#define VTY_EVENT_H

enum pty_event_code {
  PTY_CLOSED, PTY_DATA
};

enum { BUFFER_SIZE = 512 };

typedef struct pty_data {
  char * buffer;
  size_t buffer_count;
} pty_data_t;

#endif
