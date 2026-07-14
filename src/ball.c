#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ball.h"
#include "config.h"
#include "geometry.h"
#include "random.h"
#include "utils.h"

static void reset_ball(Ball *b, Seed seed) {
  b->x = CENTER_X;
  b->y = CENTER_Y;
  b->dx = rand_range(-0.5f, 0.5f, &seed);
  b->dy = rand_range(-0.5f, 0.5f, &seed);
  normalize2f(&b->dx, &b->dy);
  b->speed = BALL_START_SPEED;
  b->first_hit_done = false;
}

// Returns Scorer
// SCORER_NONE = No one scored in this frame
// SCORER_PLAYER = Local player scored
// SCORER_OPPONENT = Local opponent scored
Scorer update_ball(Ball *b, float player_y, float opponent_y, float tick_dt,
                   Seed seed) {
  Rect player = {
      .x = 0.0f,
      .y = player_y,
      .w = PADDLE_WIDTH,
      .h = PADDLE_HEIGHT,
  };

  Rect opponent = {
      .x = LOGICAL_WIDTH - PADDLE_WIDTH,
      .y = opponent_y,
      .w = PADDLE_WIDTH,
      .h = PADDLE_HEIGHT,
  };

  b->x += b->dx * b->speed * tick_dt;
  b->y += b->dy * b->speed * tick_dt;

  // Top wall
  if (b->y <= 0.0f) {
    b->y = 0.0f;
    b->dy = -b->dy;
  }

  // Bottom wall
  if (b->y >= LOGICAL_HEIGHT - BALL_HEIGHT) {
    b->y = LOGICAL_HEIGHT - BALL_HEIGHT;
    b->dy = -b->dy;
  }

  // Left paddle
  if (b->dx < 0.0f && b->x <= player.x + player.w &&
      b->x + BALL_WIDTH >= player.x && b->y + BALL_HEIGHT >= player.y &&
      b->y <= player.y + player.h) {

    b->speed += BALL_SPEED_INCREASE;
    b->x = player.x + player.w;

    float paddle_center = player.y + player.h * 0.5f;
    float ball_center = b->y + BALL_HEIGHT * 0.5f;
    float hit_pos = (ball_center - paddle_center) / (player.h * 0.5f);

    if (hit_pos < -1.0f)
      hit_pos = -1.0f;
    if (hit_pos > 1.0f)
      hit_pos = 1.0f;

    b->dx = fabsf(b->dx);

    if (fabsf(hit_pos) < 0.1f)
      b->dy *= 0.5f;
    else
      b->dy = hit_pos;

    normalize2f(&b->dx, &b->dy);

    if (!b->first_hit_done) {
      b->speed = BALL_SPEED;
      b->first_hit_done = true;
    }
  }

  // Right paddle
  if (b->dx > 0.0f && b->x + BALL_WIDTH >= opponent.x &&
      b->x <= opponent.x + opponent.w && b->y + BALL_HEIGHT >= opponent.y &&
      b->y <= opponent.y + opponent.h) {

    b->speed += BALL_SPEED_INCREASE;
    b->x = opponent.x - BALL_WIDTH;

    float paddle_center = opponent.y + opponent.h * 0.5f;
    float ball_center = b->y + BALL_HEIGHT * 0.5f;
    float hit_pos = (ball_center - paddle_center) / (opponent.h * 0.5f);

    if (hit_pos < -1.0f)
      hit_pos = -1.0f;
    if (hit_pos > 1.0f)
      hit_pos = 1.0f;

    b->dx = -fabsf(b->dx);

    if (fabsf(hit_pos) < 0.1f)
      b->dy *= 0.5f;
    else
      b->dy = hit_pos;

    float length = sqrtf(b->dx * b->dx + b->dy * b->dy);

    b->dx /= length;
    b->dy /= length;

    if (!b->first_hit_done) {
      b->speed = BALL_SPEED;
      b->first_hit_done = true;
    }
  }

  // Opponent scores
  if (b->x + BALL_WIDTH <= 0.0f) {
    reset_ball(b, seed);
    return SCORER_OPPONENT;
  }

  // Player scores
  if (b->x >= LOGICAL_WIDTH) {
    reset_ball(b, seed);
    return SCORER_PLAYER;
  }

  return SCORER_NONE;
}
