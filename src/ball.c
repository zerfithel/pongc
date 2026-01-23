#include "ball.h"
#include "config.h"
#include <stdbool.h>
#include <math.h>

// Not thread-safe, must use mutex before and after calling function
// if ball is in shared struct between threads.

// returns:
// -1 = hit top/bot wall or nothing
// 0 = you scored
// 1 = enemy scored

int update_ball(Ball *b, float paddle_y[2], float tick_dt) {
  static bool first_hit_done = false; // tracks first paddle hit in current round

  float start_x[2];
  float end_x[2];
  float start_y[2];
  float end_y[2];

  start_y[0] = paddle_y[0];
  start_y[1] = paddle_y[1];

  end_y[0] = start_y[0] + PADDLE_HEIGHT;
  end_y[1] = start_y[1] + PADDLE_HEIGHT;

  start_x[0] = 0.0f;
  start_x[1] = LOGICAL_WIDTH - PADDLE_WIDTH;
  
  end_x[0] = PADDLE_WIDTH;
  end_x[1] = LOGICAL_WIDTH;

  // new position if didn't score 
  b->x += b->dx * b->speed * tick_dt;
  b->y += b->dy * b->speed * tick_dt;

  // Ball hit top/bot wall
  if (b->y <= 0.0f) {
    b->y = 0.0f;
    b->dy = -b->dy;
  }

  if (b->y >= LOGICAL_HEIGHT - BALL_HEIGHT) {
    b->y = LOGICAL_HEIGHT - BALL_HEIGHT;
    b->dy = -b->dy;
  }

  // Ball hit left paddle
  if (b->dx < 0.0f &&
      b->x <= end_x[0] &&
      b->x + BALL_WIDTH >= start_x[0] &&
      b->y + BALL_HEIGHT >= start_y[0] &&
      b->y <= end_y[0]) 
  {
    b->x = end_x[0];

    float paddle_center = start_y[0] + PADDLE_HEIGHT / 2.0f;
    float ball_center = b->y + BALL_HEIGHT / 2.0f;
    float hit_pos = (ball_center - paddle_center) / (PADDLE_HEIGHT / 2.0f);
    if (hit_pos < -1.0f) hit_pos = -1.0f;
    if (hit_pos > 1.0f) hit_pos = 1.0f;

    b->dx = fabs(b->dx); // always right

    if (fabs(hit_pos) < 0.1f) {
      b->dy *= 0.5f; // decrease angle
    } else {
      b->dy = hit_pos;
    }

    float length = sqrt(b->dx * b->dx + b->dy * b->dy);

    if (!first_hit_done) {
      b->dx = (b->dx / length);
      b->dy = (b->dy / length);
      b->speed = BALL_SPEED;
      first_hit_done = true;
    } else {
      b->dx = b->dx / length;
      b->dy = b->dy / length;
    }
  }

  // Ball hit right paddle
  if (b->dx > 0.0f &&
      b->x + BALL_WIDTH >= start_x[1] &&
      b->x <= end_x[1] &&
      b->y + BALL_HEIGHT >= start_y[1] &&
      b->y <= end_y[1]) 
  {
    b->x = start_x[1] - BALL_WIDTH;

    float paddle_center = start_y[1] + PADDLE_HEIGHT / 2.0f;
    float ball_center = b->y + BALL_HEIGHT / 2.0f;
    float hit_pos = (ball_center - paddle_center) / (PADDLE_HEIGHT / 2.0f);
    if (hit_pos < -1.0f) hit_pos = -1.0f;
    if (hit_pos > 1.0f) hit_pos = 1.0f;

    b->dx = -fabs(b->dx); // always left

    if (fabs(hit_pos) < 0.1f) {
      b->dy *= 0.5f;
    } else {
      b->dy = hit_pos;
    }

    float length = sqrt(b->dx * b->dx + b->dy * b->dy);

    if (!first_hit_done) {
      b->dx = (b->dx / length);
      b->dy = (b->dy / length);
      b->speed = BALL_SPEED;
      first_hit_done = true;
    } else {
      b->dx = b->dx / length;
      b->dy = b->dy / length;
    }
  }

  // Ball hit left wall -> enemy scores
  if (b->x + BALL_WIDTH <= 0.0f) {
    b->x = LOGICAL_WIDTH >> 1;
    b->y = LOGICAL_HEIGHT >> 1;
    b->dx = (rand() % 2 ? 1.0f : -1.0f);
    b->dy = (rand() % 2 ? 1.0f : -1.0f);
    b->speed = BALL_START_SPEED;
    first_hit_done = false;
    return 1;
  }

  // Ball hit right wall -> player scores
  if (b->x >= LOGICAL_WIDTH) {
    b->x = LOGICAL_WIDTH >> 1;
    b->y = LOGICAL_HEIGHT >> 1;
    b->dx = (rand() % 2 ? 1.0f : -1.0f);
    b->dy = (rand() % 2 ? 1.0f : -1.0f);
    b->speed = BALL_START_SPEED;
    first_hit_done = false;
    return 0;
  }

  return -1; // no one scored
}
