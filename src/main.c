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
#endif

#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include "tinycthread.h"
#else
#include <threads.h>
#endif

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <enet/enet.h>

#include "client.h"
#include "config.h"
#include "game.h"
#include "server.h"
#include "shared.h"
#include "utils.h"

#define MIN_ARG 2

int main(int argc, char **argv) {
  int status = 0;

  // Initialise seed for rand() with the current time
  // used for ball movement & positioning in ball.c
  srand((unsigned)time(NULL));

  char ip_buffer[64] = {0};
  const char *_ip = NULL;
  enet_uint16 _port = 0;
  bool _is_server = false;

  SDL_Window *window = NULL;
  SDL_GLContext gl_ctx = NULL;
  thrd_t network_thread;
  bool thread_created = false;

  SharedData shared;
  bool sdl_ok = false;
  bool enet_ok = false;
  bool players_mtx_init = false;
  bool ball_mtx_init = false;
  bool score_mtx_init = false;

  /// parse args
  if (argc < MIN_ARG) {
    puts("Example usage:");
    printf("JOIN SERVER - %s --join ip:port\n", argv[0]);
    printf("HOST SERVER - %s --host port\n", argv[0]);
    status = 2;
    goto cleanup;
  }

  // client
  if (strcmp(argv[1], "--join") == 0) {
    if (argc < 3) {
      fprintf(stderr, "ERROR: Missing server address\n");
      status = 1;
      goto cleanup;
    }

    const char *arg = argv[2];
    arg = skip_spaces(arg);

    char *colon = strchr(arg, ':');
    if (!colon) {
      fprintf(stderr,
              "ERROR: Invalid server address, missing colon <ip:port>\n");
      status = 1;
      goto cleanup;
    }

    size_t ip_len = (size_t)(colon - arg);
    if (ip_len >= sizeof(ip_buffer)) {
      fprintf(stderr, "ERROR: IP too long\n");
      status = 1;
      goto cleanup;
    }

    memcpy(ip_buffer, arg, ip_len);
    ip_buffer[ip_len] = '\0';

    if (!valid_ipv4(ip_buffer)) {
      fprintf(stderr, "ERROR: Invalid IPv4 address: %s\n", ip_buffer);
      status = 1;
      goto cleanup;
    }

    char *endptr;
    long port_input = strtol(colon + 1, &endptr, 10);
    if (*endptr != '\0') {
      fprintf(stderr, "ERROR: Failed to convert port number\n");
      status = 1;
      goto cleanup;
    }
    if (!valid_port(port_input)) {
      fprintf(stderr, "ERROR: Invalid port: %s\n", colon + 1);
      status = 1;
      goto cleanup;
    }

    _ip = ip_buffer;
    _port = (enet_uint16)port_input;
    _is_server = false;
  }
  // server
  else if (strcmp(argv[1], "--host") == 0) {
    if (argc < 3) {
      fprintf(stderr, "ERROR: No enough arguments: missing port\n");
      status = 1;
      goto cleanup;
    }

    char *endptr;
    long port_input = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0') {
      fprintf(stderr, "ERROR: Failed to convert port number\n");
      status = 1;
      goto cleanup;
    }
    if (!valid_port(port_input)) {
      fprintf(stderr, "ERROR: Invalid port: %s\n", argv[2]);
      status = 1;
      goto cleanup;
    }

    _port = (enet_uint16)port_input;
    _is_server = true;
  } else {
    fprintf(stderr, "ERROR: Unknown argument: %s\n", argv[1]);
    status = 1;
    goto cleanup;
  }

  // sanity check
  if (!_is_server && (_ip == NULL || _port == 0)) {
    fprintf(stderr,
            "ERROR: Failed to get server IP or port\nIP: %s\nPort: %u\n", _ip,
            _port);
    status = 1;
    goto cleanup;
  }

  if (_port == 0) {
    fprintf(stderr, "ERROR: Failed to get port to host on\n");
    status = 1;
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
  window = SDL_CreateWindow(_is_server ? "PongC - Server" : "PongC - Client",
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

  if (mtx_init(&shared.players_mtx, mtx_plain) != thrd_success) {
    fprintf(stderr, "ERROR: Failed to initialize players mutex\n");
    status = 1;
    goto cleanup;
  }
  players_mtx_init = true;

  if (mtx_init(&shared.ball_mtx, mtx_plain) != thrd_success) {
    fprintf(stderr, "ERROR: Failed to initialize ball mutex\n");
    status = 1;
    goto cleanup;
  }
  ball_mtx_init = true;

  if (mtx_init(&shared.score_mtx, mtx_plain) != thrd_success) {
    fprintf(stderr, "ERROR: Failed to initialize score mutex\n");
    status = 1;
    goto cleanup;
  }
  score_mtx_init = true;

  if (_is_server) {
    if (host_server(_port) != 0) {
      fprintf(stderr, "ERROR: Failed to host server at port %d\n", _port);
      status = 1;
      goto cleanup;
    }

    if (thrd_create(&network_thread, server_loop, &shared) != thrd_success) {
      fprintf(stderr, "ERROR: Failed to create network thread for server\n");
      status = 1;
      goto cleanup;
    }
  } else {
    if (join_server(_ip, _port) != 0) {
      fprintf(stderr, "ERROR: Failed to join %s:%d\n", _ip, _port);
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
  game_loop(window, &shared, _is_server);

cleanup:
  if (thread_created) {
    thrd_join(network_thread, NULL);
  }

  if (players_mtx_init) {
    mtx_destroy(&shared.players_mtx);
  }
  if (ball_mtx_init) {
    mtx_destroy(&shared.ball_mtx);
  }
  if (score_mtx_init) {
    mtx_destroy(&shared.score_mtx);
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
