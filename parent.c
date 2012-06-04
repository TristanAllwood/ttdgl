#include <X11/Xlib.h>
#include <errno.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "parent.h"
#include "pty_event.h"
#include "render.h"
#include "ttdgl_state.h"
#include "util.h"

static int epoll_event_loop(void * data);

static void send_sdl_userevent(int code, void * packet);
static void handle_pty_closed(void);
static void handle_pty_data(char buffer[BUFFER_SIZE], size_t buffer_count);


static void sdl_render_loop(ttdgl_state_t * state);

static void handle_sdl_quit(void);
static void handle_sdl_resize(SDL_ResizeEvent * event, ttdgl_state_t * state);
static void handle_sdl_user(SDL_UserEvent * user, ttdgl_state_t * state);
static void handle_sdl_keydown(SDL_KeyboardEvent * key, ttdgl_state_t * state);
static void handle_sdl_keyup(SDL_KeyboardEvent * key, ttdgl_state_t * state);

static void handle_sdl_pty_closed(void);
static void handle_sdl_pty_write(pty_write_t * data, ttdgl_state_t * state);
static void handle_sdl_pty_set_attribute(pty_set_attributes_t * data, ttdgl_state_t * state);

void parent(pid_t child_pid, int pty_master_fd, int pty_child_fd) {
  if (close(pty_child_fd) == -1) {
    die_with_error("close [child]");
  }

  XInitThreads();

  if (SDL_Init(SDL_INIT_VIDEO) == -1) {
    die_with_error("SDL_Init");
  }

  if(atexit(SDL_Quit) != 0) {
    die_with_error("atexit");
  }

  SDL_CreateThread(epoll_event_loop, (void *) pty_master_fd);

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


  ttdgl_state_t * state = init_ttdgl_state(child_pid, pty_master_fd);

  sdl_render_loop(state);
}

static const uint16_t MAX_EVENTS_BEFORE_RENDER = 8192;

static void sdl_render_loop(ttdgl_state_t * state) {
  while(true) {
    render(state);

    uint16_t event_count = 0;

    SDL_Event event;

    while (SDL_WaitEvent(&event) == 0) {
      // TODO yukk!
      fprintf(stderr, "TODO: SDL_WaitEvent failed\n");
      sched_yield();
    }

    do {
      switch (event.type) {
        case SDL_QUIT: 
          handle_sdl_quit();
          break;
        case SDL_VIDEORESIZE:
          handle_sdl_resize(&event.resize, state);
          break;
        case SDL_USEREVENT:
          handle_sdl_user(&event.user, state);
          break;
        case SDL_KEYDOWN:
          handle_sdl_keydown(&event.key, state);
          break;
        case SDL_KEYUP:
          handle_sdl_keyup(&event.key, state);
          break;
        default:
          fprintf(stderr, "Unknown event: %x\n", event.type);
          break;
      }
    } while (++event_count < MAX_EVENTS_BEFORE_RENDER &&
              SDL_PollEvent(&event));
  }
}


static int epoll_event_loop(void * data) {
  int pty_master_fd = (int) data;

  int epoll_fd = epoll_create(1);
  if (epoll_fd == -1) {
    die_with_error("epoll_create");
  }

  struct epoll_event pty_master_event;
  pty_master_event.events = EPOLLIN;
  pty_master_event.data.fd = pty_master_fd;

  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pty_master_fd, &pty_master_event) == -1) {
    die_with_error("epoll_ctl");
  }


  static const int MAX_EVENTS=10;
  struct epoll_event events[MAX_EVENTS];

  while (true) {
    int event_count;
    do {
      event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    } while (event_count == -1 && errno == EINTR);

    if (event_count == -1) {
      die_with_error("epoll_wait");
    }

    for (int i = 0 ; i < event_count ; ++i) {

      if (events[i].data.fd == pty_master_fd) {
        char * buffer = calloc(BUFFER_SIZE, sizeof(char));
        size_t count;

        count = read(pty_master_fd, buffer, BUFFER_SIZE);

        switch (count) {
          case -1:
          case 0:
            free(buffer);
            handle_pty_closed();
            return 0;
          default:
            handle_pty_data(buffer, count);
            free(buffer);
            break;
        }
      } else {
        fprintf(stderr, "Unknown epoll file descriptor %i", events[i].data.fd);
        exit(99);
      }
    }
  }
  return 1;
}


static void handle_sdl_quit(void) {
  exit(0);
}

static void handle_sdl_resize(SDL_ResizeEvent * event, ttdgl_state_t * state) {
  surface_resize(event->w, event->h, state);
}

static void handle_sdl_user(SDL_UserEvent * user, ttdgl_state_t * state) {
  switch (user->code) {
    case PTY_CLOSED:
      handle_sdl_pty_closed();
      break;
    case PTY_WRITE:
      handle_sdl_pty_write((pty_write_t *) user->data1, state);
      break;
    case PTY_SET_ATTRIBUTE:
      handle_sdl_pty_set_attribute((pty_set_attributes_t *) user->data1, state);
      break;
    default:
      fprintf(stderr, "Unknown user code: %x\n", user->code);
      break;
  }
}

static void handle_sdl_keydown(SDL_KeyboardEvent * key_ev, ttdgl_state_t * state) {

  char out[5];
  out[1] = '\0';
  SDLKey key = key_ev->keysym.sym;

  if (key == SDLK_RETURN) {
      out[0] = '\r';
  } else if (key >= SDLK_a && key <= SDLK_z) {
      out[0] = 'a' + key - SDLK_a;
  } else {
    fprintf(stderr, "TODO: handle keydown: %x\n", key);
    return;
  }

  put_char(out, state);
  write(state->pty_master_fd, out, 2);
  // TODO error check the write call.
}

static void handle_sdl_keyup(SDL_KeyboardEvent * key, ttdgl_state_t * state) {
  // TODO
}

static void handle_sdl_pty_closed(void) {
  handle_sdl_quit();
}

static void handle_sdl_pty_write(pty_write_t * data, ttdgl_state_t * state) {
  put_char(data->nt_unicode_char, state);
  free(data);
}

static void handle_sdl_pty_set_attribute(pty_set_attributes_t * data, ttdgl_state_t * state) {
  /* check for xterm 256 colours */

  for (int i = 0 ; i < data->arg_count ; i++) {
    switch (data->attr_codes[i]) {
      case 0:
        state->current_attrs.attr_flags = 0;
        state->current_attrs.foreground_colour = 0x000000;
        break;
      case 1:
        state->current_attrs.attr_flags |= ATTR_BOLD;
        break;
      case 5:
        state->current_attrs.attr_flags |= ATTR_BLINK;
        break;
      case 33:
        state->current_attrs.foreground_colour = 0xff0000;
        break;
      case 36:
        state->current_attrs.foreground_colour = 0x00ffff;
        break;
      case 38:
        state->current_attrs.attr_flags |= ATTR_UNDERSCORE;
        break;
      case 94:
        state->current_attrs.foreground_colour = 0x00ff00;
        break;
      default:
        fprintf(stderr, "TODO: unknown attr code: %ld\n", data->attr_codes[i]);
        break;
    }
  }

  free(data);
}


static void handle_pty_closed(void) {
  send_sdl_userevent(PTY_CLOSED, NULL);
}

static void push_pty_write(char nt_unicode_char[5]) {
  pty_write_t * packet = malloc(sizeof(pty_write_t));
  memcpy(packet->nt_unicode_char, nt_unicode_char, 5);
  send_sdl_userevent(PTY_WRITE, packet);
}

static void send_sdl_userevent(int code, void * packet) {
  SDL_Event user_event;
  user_event.type = SDL_USEREVENT;
  user_event.user.code = code;
  user_event.user.data1 = packet;
  user_event.user.data2 = NULL;

  while (SDL_PushEvent(&user_event) == -1) {
    // TODO yuk!
    sched_yield();
  }
}

static int parse_command(char * buffer, size_t buffer_count) {
  int position = 0;

  if (position >= buffer_count) {
    printf("TODO: parse_command runs");
    return 0;
  }

  // TODO: check position doesn't drop off here.
  switch (buffer[position++]) {
    case '[': /* CSI */
    {
      /* check for ? */
      // bool q = false; - unused
      if (buffer[position] == '?') {
        // q = true; - unused
        position++;
      }

      /* read sequence of numbers, seperated by ';' */
      long int args[16];
      int arg_count = 0;

      char command;
      bool hit_something;
 
      while (position < buffer_count) {
        char c_char = buffer[position++];

        if (c_char == ';') {
          if (!hit_something) {
            args[arg_count++] = 0;
          }
          hit_something = false;
          // do nothing
        } else if (c_char >= '0' && c_char <= '9') {
          hit_something = true;
          char ** end_ptr = malloc(sizeof(char *));
          char * start_loc = &buffer[position-1];
          args[arg_count++] = strtol(start_loc, end_ptr, 10);
          position += (*end_ptr - start_loc - 1);
          free(end_ptr);

          if (arg_count >= 16) {
            fprintf(stderr, "TODO: handle more than 16 numbers in a control code");
            break;
          }
        } else {
          if (!hit_something) {
            args[arg_count++] = 0;
          }
            /* read final character */
          command = c_char;
          break;
        }
      }

      switch (command) {
        case 'm': /* set attributes */ 
        {
          pty_set_attributes_t * packet = malloc(sizeof(pty_set_attributes_t));
          memcpy(packet->attr_codes, args, sizeof(args));
          packet->arg_count = arg_count;
          send_sdl_userevent(PTY_SET_ATTRIBUTE, packet);
          break;
        }
        default:
        {

          fprintf(stderr, "TODO: unknown command: %c\n", command);
          break;
        }
      }
      break;
    }

    case ']': /* OSC */
    {
      break;
    }
  }
  return position;
}

static void handle_pty_data(char * buffer, size_t buffer_count) {

  int position = 0;

  while(position < buffer_count) {
    char byte = buffer[position++];

    if (byte == 0x1B) {
      position += parse_command(&buffer[position], buffer_count - position);

    } else if ((byte & 0x80) == 0x00) {
      // normal char 
      char out[5];
      out[0] = byte;
      out[1] = '\0';
      push_pty_write(out);

    } else if ((byte & 0xE0) == 0x60) {
      // 2-byte utf-8 
      if (position >= buffer_count-2) {
        fprintf(stderr, "TODO: handle 2-byte utf8 excess runs\n");
        continue;
      }

      char out[5];
      out[0] = byte;
      out[1] = buffer[position++];
      out[2] = '\0';
      push_pty_write(out);

    } else if ((byte & 0xF0) == 0xE0) {
      if (position >= buffer_count-3) {
        fprintf(stderr, "TODO: handle 3-byte utf8 excess runs\n");
        continue;
      }

      char out[5];
      out[0] = byte;
      out[1] = buffer[position++];
      out[2] = buffer[position++];
      out[3] = '\0';
      push_pty_write(out);

    } else if ((byte & 0xF8) == 0xC0) {
      if (position >= buffer_count-4) {
        fprintf(stderr, "TODO: handle 4-byte utf8 excess runs\n");
        continue;
      }

      char out[5];
      out[0] = byte;
      out[1] = buffer[position++];
      out[2] = buffer[position++];
      out[3] = buffer[position++];
      out[4] = '\0';
      push_pty_write(out);
    } else {
      fprintf(stderr, "Unknown byte: %c\n", byte);
    }
  }
}
