#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include "external/tinycthread.h"
#else
#include <threads.h>
#endif

#include "config.h"
#include "game.h"
#include "geometry.h"
#include "random.h"
#include "shaders.h"
#include "shared.h"
#include "utils.h"

// Game thread
// Is responsible for game logic and rendering
void game_loop(SDL_Window *window, SharedData *shared, bool server) {
  Seed seed = make_seed();

  // quad for paddles and VAO/VBO
  float quad[] = {0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1};

  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
  glEnableVertexAttribArray(0);

  // create shaders programs
  GLuint ball_prog = load_shader(ball_vertex_shader, ball_frag_shader);
  GLuint paddle_prog = load_shader(paddle_vertex_shader, paddle_frag_shader);
  if (ball_prog == 0 || paddle_prog == 0) {
    // Error log is printed out by load_shader()
    return;
  }

  // 2D orthographic projection
  float proj[16];
  ortho(proj, 0, LOGICAL_WIDTH, LOGICAL_HEIGHT, 0);

  // ball
  GLint uProj_ball = glGetUniformLocation(ball_prog, "uProj");
  GLint uPos_ball = glGetUniformLocation(ball_prog, "uPos");
  GLint uSize_ball = glGetUniformLocation(ball_prog, "uSize");

  // paddle
  GLint uProj_paddle = glGetUniformLocation(paddle_prog, "uProj");
  GLint uPos_paddle = glGetUniformLocation(paddle_prog, "uPos");
  GLint uSize_paddle = glGetUniformLocation(paddle_prog, "uSize");
  GLint uColor_paddle = glGetUniformLocation(paddle_prog, "uColor");

  float player_y = 0.0f;
  float opponent_y = 0.0f;
  Vec2f ball = {
      .x = 0.0f,
      .y = 0.0f,
  };

  const double tick_dt = 1.0 / GAME_TPS;
  Uint64 prev_counter = SDL_GetPerformanceCounter();
  double accumulator = 0.0;

  SDL_Event event;
  while (atomic_load(&shared->running)) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          int drawable_x = 0;
          int drawable_y = 0;

          int drawable_width = 0;
          int drawable_height = 0;
          SDL_GL_GetDrawableSize(window, &drawable_width, &drawable_height);

          glViewport(drawable_x, drawable_y, drawable_width, drawable_height);
        }
        break;

      case SDL_QUIT:
        // finish this iteration and end this loop
        // also tell the network thread to end its loop
        atomic_store(&shared->running, false);
        break;
      }
    }

    Uint64 now = SDL_GetPerformanceCounter();
    double frame_time =
        (double)(now - prev_counter) / (double)SDL_GetPerformanceFrequency();
    prev_counter = now;

    const double FRAME_TIME_CLAMP = 0.25;
    if (frame_time > FRAME_TIME_CLAMP) {
      frame_time = FRAME_TIME_CLAMP;
    }
    accumulator += frame_time;

    const Uint8 *state = SDL_GetKeyboardState(NULL);
    float dy = 0.0f;
    if (state[SDL_SCANCODE_W]) {
      dy -= 1.0f;
    }
    if (state[SDL_SCANCODE_S]) {
      dy += 1.0f;
    }

    // Ticks logic
    mtx_lock(&shared->mtx);
    while (accumulator >= tick_dt) {
      // Server
      if (server) {
        int scorer = update_ball(&shared->ball, player_y, opponent_y,
                                 (float)tick_dt, seed);
        if (scorer == SCORER_PLAYER) {
          shared->player_score += 1;
        } else if (scorer == SCORER_OPPONENT) {
          shared->opponent_score += 1;
        }
      } else { // Client
        update_ball(&shared->ball, player_y, opponent_y, (float)tick_dt, seed);
      }

      // calculate new player pos
      shared->player_y += dy * PADDLE_SPEED * (float)tick_dt;
      shared->player_y =
          clamp(shared->player_y, 0.0f, LOGICAL_HEIGHT - PADDLE_HEIGHT);
      player_y = shared->player_y;
      opponent_y = shared->opponent_y;

      // new ball position (info from network thread)
      ball.x = shared->ball.x;
      ball.y = shared->ball.y;

      accumulator -= tick_dt;
    }
    mtx_unlock(&shared->mtx);

    /// render
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(vao);

    // Left paddle (me)
    glUseProgram(paddle_prog);
    glUniformMatrix4fv(uProj_paddle, 1, GL_FALSE, proj);
    glUniform2f(uPos_paddle, 0.0f, player_y);
    glUniform2f(uSize_paddle, PADDLE_WIDTH, PADDLE_HEIGHT);
    glUniform3f(uColor_paddle, 1.0f, 0.0f, 0.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Right paddle (him)
    glUseProgram(paddle_prog);
    glUniformMatrix4fv(uProj_paddle, 1, GL_FALSE, proj);
    glUniform2f(uPos_paddle, (LOGICAL_WIDTH - PADDLE_WIDTH), opponent_y);
    glUniform2f(uSize_paddle, PADDLE_WIDTH, PADDLE_HEIGHT);
    glUniform3f(uColor_paddle, 0.0f, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Ball
    glUseProgram(ball_prog);
    glUniformMatrix4fv(uProj_ball, 1, GL_FALSE, proj);
    glUniform2f(uPos_ball, ball.x, ball.y);
    glUniform2f(uSize_ball, BALL_WIDTH, BALL_HEIGHT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    SDL_GL_SwapWindow(window);
  }

  // Cleanup shaders and VBO/VAO
  glDeleteProgram(ball_prog);
  glDeleteProgram(paddle_prog);
  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);

  return;
}
