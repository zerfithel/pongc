/*
The MIT License (MIT)

Copyright © 2026 Zerfithel

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifdef _WIN32
  #define SDL_MAIN_HANDLED
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <time.h>
#ifdef _WIN32
  #include "../external/tinycthread.h"
#else
  #include <threads.h>
#endif

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <enet/enet.h>

#include "game.h"
#include "client.h"
#include "server.h"
#include "utils.h"
#include "shared.h"
#include "config.h"

constexpr int MIN_ARG = 2;

int main(int argc, char **argv) {
  srand(time(NULL));

  char ip_buffer[64] = {0};
  const char *_ip   = NULL;
  enet_uint16 _port = 0;
  bool _is_server = false;

  /// parse args
  if (argc < MIN_ARG) {
    puts("Example usage:");
    printf("JOIN SERVER - %s --join ip:port\n", argv[0]);
    printf("HOST SERVER - %s --host port\n", argv[0]);
    return 2;
  }

  // client
  if (strcmp(argv[1], "--join") == 0) {
    if (argc < 3) {
      fprintf(stderr, "ERROR: Missing server address\n");
      return 1;
    }

    const char *arg = argv[2];
    arg = skip_spaces(arg);

    char *colon = strchr(arg, ':');
    if (!colon) {
      fprintf(stderr, "ERROR: Invalid server address, missing colon <ip:port>\n");
      return 1;
    }

    size_t ip_len = colon - arg;
    if (ip_len >= sizeof(ip_buffer)) {
      fprintf(stderr, "ERROR: IP too long\n");
      return 1;
    }

    memcpy(ip_buffer, arg, ip_len);
    ip_buffer[ip_len] = '\0';

    if (!valid_ipv4(ip_buffer)) {
      fprintf(stderr, "ERROR: Invalid IPv4 address: %s\n", ip_buffer);
      return 1;
    }

    int int_port = atoi(colon + 1);
    if (!valid_port(int_port)) {
      fprintf(stderr, "ERROR: Invalid port: %s\n", colon + 1);
      return 1;
    }

    _ip = ip_buffer;
    _port = (enet_uint16)int_port;
    _is_server = false;
  }
  // server
  else if (strcmp(argv[1], "--host") == 0) {
    if (argc < 3) {
      fprintf(stderr, "ERROR: No enough arguments: missing port\n");
      return 1;
    }
    _port = (enet_uint16)atoi(argv[2]);
    if (!valid_port(_port)) {
      fprintf(stderr, "ERROR: Invalid port: %s\n", argv[2]);
      return 1;
    }
    _is_server = true;
  }
  else {
    fprintf(stderr, "ERROR: Unknown argument: %s\n", argv[1]);
    return 1;
  }

  // sanity check
  if (!_is_server &&
      (
        _ip == NULL ||
        _port == 0
      )
     )
  {
    fprintf(stderr, "ERROR: Failed to get server IP or port\nIP: %s\nPort: %u\n", _ip, _port);
    return 1;
  }
  else {
    if (_port == 0) {
      fprintf(stderr, "ERROR: Failed to get port to host on\n");
      return 1;
    }
  }

  // initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "ERROR: Failed to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                      SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // create window
  SDL_Window *window = SDL_CreateWindow(
    _is_server ? "PongC - Server" : "PongC - Client",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    WINDOW_FLAGS
  );
  if (!window) {
    fprintf(stderr, "Error: Failed to create window: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  // create GL Context
  SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1);

  // initialize GLEW
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "ERROR: Failed to initialize glew\n");
    SDL_GL_DeleteContext(gl_ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // initialize enet
  if (enet_initialize() != 0) {
    fprintf(stderr, "ERROR: Failed to initialize enet\n");
    SDL_GL_DeleteContext(gl_ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  // Shared structure and mutex
  SharedData shared;
  thrd_t network_thread;
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
  if (mtx_init(&shared.players_mtx, mtx_plain) != thrd_success ||
      mtx_init(&shared.ball_mtx, mtx_plain) != thrd_success ||
      mtx_init(&shared.score_mtx, mtx_plain) != thrd_success) 
  {
    fprintf(stderr, "ERROR: Failed to initialize mutex\n");
    SDL_GL_DeleteContext(gl_ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  if (_is_server) {
    host_server(_port);
    if (thrd_create(&network_thread, server_loop, &shared) != thrd_success) {
      fprintf(stderr, "ERROR: Failed to create network thread for server\n");
      SDL_GL_DeleteContext(gl_ctx);
      SDL_DestroyWindow(window);
      SDL_Quit();
      enet_deinitialize();
      return 1;
    }
  }
  else {
    join_server(_ip, _port);
    if (thrd_create(&network_thread, client_loop, &shared) != thrd_success) {
      fprintf(stderr, "ERROR: Failed to create network thread for client\n");
      SDL_GL_DeleteContext(gl_ctx);
      SDL_DestroyWindow(window);
      SDL_Quit();
      enet_deinitialize();
      return 1;
    }
  }

  // start game loop
  game_loop(window, &shared, _is_server);

  // cleanup and exit
  thrd_join(network_thread, NULL);
  mtx_destroy(&shared.players_mtx);
  mtx_destroy(&shared.ball_mtx);
  mtx_destroy(&shared.score_mtx);
  SDL_GL_DeleteContext(gl_ctx);
  SDL_DestroyWindow(window);
  SDL_Quit();
  enet_deinitialize();
  return 0;
}
