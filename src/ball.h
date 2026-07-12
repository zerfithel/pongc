#pragma once

#include <stdint.h>

typedef enum : int {
  SCORER_NONE = 0,
  SCORER_PLAYER = 1,
  SCORER_OPPONENT = 2,
} Scorer;

typedef struct {
  float x, y;   // Ball position
  float dx, dy; // Moving direction
  float speed;  // Movement speed
} Ball;

/* Not thread-safe, must use mutex before and after calling function
 * if ball is in shared struct between threads.

 * returns:
 * -1 = hit top/bot wall or nothing
 * 0 = you scored
 * 1 = enemy scored
*/

// The returns (0 and 1) are indexes in score[2] array to avoid big if block
Scorer update_ball(Ball *b, float paddle_y[2], float tick_dt, uint32_t seed);
