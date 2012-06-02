#include <unistd.h>
#include <stdbool.h>
#include <sys/epoll.h>

#include "parent.h"
#include "pty_event.h"
#include "render.h"
#include "ttdgl_state.h"
#include "util.h"

static int parent_pty_event_loop(void * data);

static void handle_pty_closed(void);
static void handle_pty_data(char buffer[BUFFER_SIZE], size_t buffer_count);


static void main_loop(ttdgl_state_t * state);

static void handle_sdl_quit(void);
static void handle_sdl_resize(SDL_ResizeEvent * event, ttdgl_state_t * state);
static void handle_sdl_user(SDL_UserEvent * user, ttdgl_state_t * state);
static void handle_sdl_keydown(SDL_KeyboardEvent * key, ttdgl_state_t * state);
static void handle_sdl_keyup(SDL_KeyboardEvent * key, ttdgl_state_t * state);

static void handle_sdl_pty_closed(void);
static void handle_sdl_pty_data(pty_data_t * data, ttdgl_state_t * state);



void parent(int pty_master_fd, int pty_child_fd) {
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

  ttdgl_state_t * state = init_ttdgl_state(pty_master_fd);

  main_loop(state);

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


static int parent_pty_event_loop(void * data) {
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
  int event_count;

  while((event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1)) != -1) {
    for(int i = 0 ; i < event_count ; ++i) {

      if (events[i].data.fd == pty_master_fd) {
        char * buffer = calloc(BUFFER_SIZE, sizeof(char));
          // free: either here, or in the main event loop

        size_t count;

        count = read(pty_master_fd, buffer, BUFFER_SIZE);

        switch (count) {
          case -1:
            die_with_error("read");
            break;
          case 0:
            handle_pty_closed();
            free(buffer);
            break;
          default:
            handle_pty_data(buffer, count);
            break;
        }
      } else {
        fprintf(stderr, "Unknown epoll file descriptor %i", events[i].data.fd);
        exit(99);
      }
    }
  }

  die_with_error("epoll_wait");
  
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
    case PTY_DATA:
      handle_sdl_pty_data((pty_data_t *) user->data1, state);
      break;
    default:
      fprintf(stderr, "Unknown user code: %x\n", user->code);
      break;
  }
}

static void handle_sdl_keydown(SDL_KeyboardEvent * key, ttdgl_state_t * state) {
  // TODO
}

static void handle_sdl_keyup(SDL_KeyboardEvent * key, ttdgl_state_t * state) {
  // TODO
}

static void handle_sdl_pty_closed(void) {
  handle_sdl_quit();
}

static void handle_sdl_pty_data(pty_data_t * data, ttdgl_state_t * state) {
  // TODO: must free or pass along buffer and data pointers.
}


static void handle_pty_closed(void) {
  SDL_Event user_event;
  user_event.type = SDL_USEREVENT;
  user_event.user.code = PTY_CLOSED;
  user_event.user.data1 = NULL;
  user_event.user.data2 = NULL;

  SDL_PushEvent(&user_event);
}

static void handle_pty_data(char * buffer, size_t buffer_count) {
  pty_data_t * pty_data = malloc(sizeof(pty_data));

  pty_data->buffer = buffer;
  pty_data->buffer_count = buffer_count;

  SDL_Event user_event;
  user_event.type = SDL_USEREVENT;
  user_event.user.code = PTY_DATA;
  user_event.user.data1 = (void *) pty_data;
  user_event.user.data2 = NULL;

  SDL_PushEvent(&user_event);
}
