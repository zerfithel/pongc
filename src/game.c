/*
The MIT License (MIT)

Copyright © 2026 Zerfithel

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "game.h"
#include "shared.h"
#include "config.h"
#include "utils.h"

// game loop (main thread)
void game_loop(SDL_Window *window, SDL_Renderer *renderer, SharedData *shared) {
  (void)window;
  SDL_Event event;

  const double tick_dt = 1.0 / GAME_TPS;
  Uint64 prev_counter = SDL_GetPerformanceCounter();
  double accumulator = 0.0;

  float y[2] = {0.0f, 0.0f};
  float ball_xy[2] = {0.0f, 0.0f}; // ball_xy[0] = x, ball_xy[1] = y

  while (atomic_load(&shared->running)) {
    // handle SDL events
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        // finish this iteration and break and tell network thread to end its work
        atomic_store(&shared->running, false);
      }
    }

    // time
    Uint64 now = SDL_GetPerformanceCounter();
    double frame_time = (double)(now - prev_counter) / SDL_GetPerformanceFrequency();
    prev_counter = now;

    if (frame_time > 0.25) {
      frame_time = 0.25;
    }

    accumulator += frame_time;

    // handle input
    float dy = 0.0f;
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    if (state[SDL_SCANCODE_W]) {
      dy -= 1.0f;
    }
    if (state[SDL_SCANCODE_S]) {
      dy += 1.0f;
    }

    // ticks
    while (accumulator >= tick_dt) {
      mtx_lock  (&shared->players_mtx);
      {
        // calculate my new pos and save mine and his pos in y[]
        shared->y[0] += dy * PADDLE_SPEED * (float)tick_dt;
        shared->y[0] = clamp(shared->y[0], 0.0f, LOGICAL_HEIGHT - PADDLE_HEIGHT);
        y[0] = shared->y[0];
        y[1] = shared->y[1];
      }
      mtx_unlock(&shared->players_mtx);

      mtx_lock  (&shared->ball_mtx);
      {
        ball_xy[0] = shared->ball.x;
        ball_xy[1] = shared->ball.y;
      }
      mtx_unlock(&shared->ball_mtx);

      accumulator -= tick_dt;
    }

    /// render

    int paddle_width  = PADDLE_WIDTH;
    int paddle_height = PADDLE_HEIGHT;
    
    // paddles
    SDL_Rect me  = {
      .x = 0,
      .y = (int)y[0],
      .w = paddle_width,
      .h = paddle_height
    };
    SDL_Rect him = {
      .x = LOGICAL_WIDTH - paddle_width,
      .y = (int)y[1],
      .w = paddle_width,
      .h = paddle_height
    };

    // ball
    SDL_Rect ball = {
      .x = (int)ball_xy[0],
      .y = (int)ball_xy[1],
      .w = BALL_WIDTH,
      .h = BALL_HEIGHT
    };

    // background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // me
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &me);

    // him
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRect(renderer, &him);

    // ball
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &ball);

    SDL_RenderPresent(renderer);
  }

  return;
}
