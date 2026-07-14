#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "random.h"

typedef enum : int {
  SCORER_NONE = 0,
  SCORER_PLAYER = 1,
  SCORER_OPPONENT = 2,
} Scorer;

typedef struct {
  float x, y;
  float dx, dy;
  float speed;
  bool first_hit_done;
} Ball;

/* Not thread-safe, must use mutex before and after calling function
 * if ball is in shared struct between threads.

 * returns:
 * -1 = hit top/bot wall or nothing
 * 0 = you scored
 * 1 = enemy scored
*/

// The returns (0 and 1) are indexes in score[2] array to avoid big if block
Scorer update_ball(Ball *b, float player_y, float opponent_y, float tick_dt,
                   Seed seed);
