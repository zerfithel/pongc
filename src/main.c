/*
The MIT License (MIT)

Copyright © 2026 Zerfithel

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifdef _WIN32
#define SDL_MAIN_HANDLED
#endif // ifdef _WIN32

#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include "external/tinycthread.h"
#else
#include <threads.h>
#endif // ifdef _WIN32

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <enet/enet.h>

#include "client.h"
#include "cmdline.h"
#include "config.h"
#include "game.h"
#include "server.h"
#include "shared.h"
#include "utils.h"

#define MIN_ARG 2

/*
 * Print help message
 */
static void usage(void) {
  printf("Usage: %s [OPTIONS...]\n\n"
         "OPTIONS\n"
         " --host, -h   Host server\n"
         " --join, -j   Join server\n"
         " --ip,   -i   Provide IP\n"
         " --port, -p   Provide port\n\n"
         "EXAMPLES\n"
         " %s --join --ip 127.0.0.1 --port 4444\n"
         " %s --host -i 127.0.0.1 --port 4444\n"
         " %s -j -i 127.0.0.1 -p 5999\n",
         PROGRAM_NAME, PROGRAM_NAME, PROGRAM_NAME, PROGRAM_NAME);
}

/*
 * Program arguments handlers
 * Argument handler types definition is in `cmdline.h`
 */

// --host, -h
static int handle_host(Args *args, const char *value) {
  (void)value;
  args->is_server = true;
  return 0;
}

// --join, -j
static int handle_join(Args *args, const char *value) {
  (void)value;
  args->is_server = false;
  return 0;
}

// --ip, -i
static int handle_ip(Args *args, const char *value) {
  if (!value)
    return 1;
  if (!valid_ipv4(value)) {
    return 1;
  }
  size_t ip_size = sizeof(args->ip);
  int len = snprintf(args->ip, ip_size, "%s", value);
  if (len < 0) {
    fprintf(stderr, "ERROR: Formatting IP failed\n");
    return 1;
  }

  if ((size_t)len >= ip_size) {
    fprintf(stderr, "ERROR: IP string truncated, exiting...\n");
    return 1;
  }

  return 0;
}

// --port, -p
static int handle_port(Args *args, const char *value) {
  if (!value)
    return 1;

  char *endptr = NULL;
  long port = strtol(value, &endptr, 10);
  if (*endptr != '\0') {
    fprintf(stderr, "ERROR: Failed to convert port\n");
    return 1;
  }
  if (!valid_port(port)) {
    fprintf(stderr, "ERROR: Port is invalid\n");
    return 1;
  }

  args->port = (enet_uint16)port;
  return 0;
}

/*
 * Program entry point
 * Initializes environment for game and runs threads
 */
int main(int argc, char **argv) {
  int status = 0;

  // Initialise seed for rand() with the current time
  // used for ball movement & positioning in ball.c
  srand((unsigned)time(NULL));

  SDL_Window *window = NULL;
  SDL_GLContext gl_ctx = NULL;
  thrd_t network_thread;
  bool thread_created = false;

  SharedData shared;
  bool sdl_ok = false;
  bool enet_ok = false;
  bool mtx_ok = false;

  // cmdline options
  const Option options[] = {
      {
          .handler = handle_host,
          .long_opt = "host",
          .short_opt = 'h',
          .argument = false,
      },
      {
          .handler = handle_join,
          .long_opt = "join",
          .short_opt = 'j',
          .argument = false,
      },
      {
          .handler = handle_ip,
          .long_opt = "ip",
          .short_opt = 'i',
          .argument = true,
      },
      {
          .handler = handle_port,
          .long_opt = "port",
          .short_opt = 'p',
          .argument = true,
      },
      /* DO NOT NULL TERMINATE THIS STRUCTURE, THIS WILL CAUSE SEGFAULT AT
      find_option() IN cmdline.c
      {
          .handler = NULL,
          .long_opt = NULL,
          .short_opt = '\0',
          .argument = false,
      },
      */
  };
  const int options_count = sizeof(options) / sizeof(options[0]);

  // Parse arguments
  Args args = {0};
  if (parse_args(&args, argc, argv, options, options_count) != 0) {
    usage();
    status = 2;
    goto cleanup;
  }

  if (args.ip[0] == '\0' || args.port == 0) {
    usage();
    status = 2;
    goto cleanup;
  }

  // initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "ERROR: Failed to initialize SDL: %s\n", SDL_GetError());
    status = 1;
    goto cleanup;
  }
  sdl_ok = true;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // create window
  window =
      SDL_CreateWindow(args.is_server ? "PongC - Server" : "PongC - Client",
                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FLAGS);

  if (!window) {
    fprintf(stderr, "Error: Failed to create window: %s\n", SDL_GetError());
    status = 1;
    goto cleanup;
  }

  // create GL Context
  gl_ctx = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1);

  // initialize GLEW
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "ERROR: Failed to initialize glew\n");
    status = 1;
    goto cleanup;
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // initialize enet
  if (enet_initialize() != 0) {
    fprintf(stderr, "ERROR: Failed to initialize enet\n");
    status = 1;
    goto cleanup;
  }
  enet_ok = true;

  // Shared structure and mutex
  shared.y[0] = 0.0f;
  shared.y[1] = 0.0f;
  shared.score[0] = 0;
  shared.score[1] = 0;
  shared.ball.x = LOGICAL_WIDTH >> 1;
  shared.ball.y = LOGICAL_HEIGHT >> 1;
  shared.ball.dx = 0.0f;
  shared.ball.dy = 0.0f;
  shared.ball.speed = BALL_START_SPEED;

  atomic_store(&shared.running, true);

  if (mtx_init(&shared.mtx, mtx_plain) != thrd_success) {
    fprintf(stderr, "ERROR: Failed to initialize players mutex\n");
    status = 1;
    goto cleanup;
  }
  mtx_ok = true;

  if (args.is_server) {
    if (host_server(args.ip, args.port) != 0) {
      fprintf(stderr, "ERROR: Failed to host server at port %d\n", args.port);
      status = 1;
      goto cleanup;
    }

    if (thrd_create(&network_thread, server_loop, &shared) != thrd_success) {
      fprintf(stderr, "ERROR: Failed to create network thread for server\n");
      status = 1;
      goto cleanup;
    }
  } else {
    if (join_server(args.ip, args.port) != 0) {
      fprintf(stderr, "ERROR: Failed to join %s:%d\n", args.ip, args.port);
      status = 1;
      goto cleanup;
    }

    if (thrd_create(&network_thread, client_loop, &shared) != thrd_success) {
      fprintf(stderr, "ERROR: Failed to create network thread for client\n");
      status = 1;
      goto cleanup;
    }
  }

  thread_created = true;

  // start game loop
  game_loop(window, &shared, args.is_server);

cleanup:
  if (thread_created) {
    thrd_join(network_thread, NULL);
  }

  if (mtx_ok) {
    mtx_destroy(&shared.mtx);
  }

  if (gl_ctx) {
    SDL_GL_DeleteContext(gl_ctx);
  }

  if (window) {
    SDL_DestroyWindow(window);
  }

  if (sdl_ok) {
    SDL_Quit();
  }

  if (enet_ok) {
    enet_deinitialize();
  }

  return status;
}
