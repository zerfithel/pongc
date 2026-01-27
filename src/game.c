/*
The MIT License (MIT)

Copyright © 2026 Zerfithel

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <threads.h>

#include "game.h"
#include "shared.h"
#include "config.h"
#include "utils.h"
#include "shaders.h"

// game loop ran in main thread
void game_loop(SDL_Window *window, SharedData *shared, bool server) {
  // quad for paddles and VAO/VBO 
  float quad[] = 
  {
    0,0, 1,0, 1,1, 
    0,0, 1,1, 0,1
  };

  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
  glEnableVertexAttribArray(0);

  // create shaders programs
  GLuint ball_prog   = load_shader(ball_vertex_shader, ball_frag_shader);
  GLuint paddle_prog = load_shader(paddle_vertex_shader, paddle_frag_shader);
  if (ball_prog == 0 || paddle_prog == 0) {
    // Error log is printed out by load_shader()
    return;
  }

  // 2D orthographic projection
  float proj[16]; 
  ortho(proj, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT, 0);

  // ball
  GLint uProj_ball    = glGetUniformLocation(ball_prog, "uProj");
  GLint uPos_ball     = glGetUniformLocation(ball_prog, "uPos");
  GLint uSize_ball    = glGetUniformLocation(ball_prog, "uSize");

  // paddle
  GLint uProj_paddle  = glGetUniformLocation(paddle_prog, "uProj");
  GLint uPos_paddle   = glGetUniformLocation(paddle_prog, "uPos");
  GLint uSize_paddle  = glGetUniformLocation(paddle_prog, "uSize");
  GLint uColor_paddle = glGetUniformLocation(paddle_prog, "uColor");

  float y[2] = {0.0f,0.0f};        // y[0] = my pos, y[1] = his pos
  float ball_pos[2] = {0.0f,0.0f}; // ball x and y pos

  // TPS
  const double tick_dt = 1.0 / GAME_TPS;
  Uint64 prev_counter = SDL_GetPerformanceCounter();
  double accumulator = 0.0;

  SDL_Event event;

  // Main loop
  while (atomic_load(&shared->running)) {
    // handle SDL events
    while (SDL_PollEvent(&event)) {
      // handle closed window/program
      if (event.type == SDL_QUIT) {
        // finish this iteration and end this loop
        // also tell the network thread to end its loop
        atomic_store(&shared->running, false);
      }
    }

    // Time
    Uint64 now = SDL_GetPerformanceCounter();
    double frame_time = (double)(now - prev_counter)/SDL_GetPerformanceFrequency();
    prev_counter = now;

    if (frame_time > 0.25) {
      frame_time = 0.25;
    }
    accumulator += frame_time;

    // Handle input
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    float dy = 0.0f;
    if (state[SDL_SCANCODE_W]) {
      dy -= 1.0f;
    }
    if (state[SDL_SCANCODE_S]) {
      dy += 1.0f;
    }

    // Ticks logic
    while (accumulator >= tick_dt) {
      if (!server) {
        mtx_lock(&shared->ball_mtx);
        {
          shared->ball.x += shared->ball.dx * shared->ball.speed * (float)tick_dt;
          shared->ball.y += shared->ball.dy * shared->ball.speed * (float)tick_dt;
        }
        mtx_unlock(&shared->ball_mtx);
      }
      mtx_lock(&shared->players_mtx);
      {
        // calculate new player pos
        shared->y[0] += dy * PADDLE_SPEED * (float)tick_dt;
        shared->y[0] = clamp(shared->y[0], 0.0f, LOGICAL_HEIGHT-PADDLE_HEIGHT);
        y[0] = shared->y[0];
        y[1] = shared->y[1];
      }
      mtx_unlock(&shared->players_mtx);

      // new ball position (info from network thread)
      mtx_lock(&shared->ball_mtx);
      {
        ball_pos[0] = shared->ball.x;
        ball_pos[1] = shared->ball.y;
      }
      mtx_unlock(&shared->ball_mtx);

      accumulator -= tick_dt;
    }

    /// render
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(vao);

    // Left paddle (me)
    glUseProgram(paddle_prog);
    glUniformMatrix4fv(uProj_paddle, 1, GL_FALSE, proj);
    glUniform2f(uPos_paddle, 0.0f, y[0]);
    glUniform2f(uSize_paddle, PADDLE_WIDTH, PADDLE_HEIGHT);
    glUniform3f(uColor_paddle, 1.0f, 0.0f, 0.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Right paddle (him)
    glUseProgram(paddle_prog);
    glUniformMatrix4fv(uProj_paddle, 1, GL_FALSE, proj);
    glUniform2f(uPos_paddle, (LOGICAL_WIDTH - PADDLE_WIDTH), y[1]);
    glUniform2f(uSize_paddle, PADDLE_WIDTH, PADDLE_HEIGHT);
    glUniform3f(uColor_paddle, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Ball
    glUseProgram(ball_prog);
    glUniformMatrix4fv(uProj_ball, 1, GL_FALSE, proj);
    glUniform2f(uPos_ball, ball_pos[0], ball_pos[1]);
    glUniform2f(uSize_ball, BALL_WIDTH, BALL_HEIGHT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    SDL_GL_SwapWindow(window);
  }

  // Cleaunp shaders and VBO/VAO
  glDeleteProgram(ball_prog);
  glDeleteProgram(paddle_prog);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);

  return;
}

