#pragma once

#include <stdalign.h>
#include <stdatomic.h>

#ifdef _WIN32
#include "external/tinycthread.h"
#else
#include <threads.h>
#endif // ifdef _WIN32

#include "ball.h"

/*
 * Structure SharedData: Shared data between threads (main, network threads),
 * contains game state and atomic_bool running
 *
 * To prevent false sharing (cache miss) i added padding (64 bytes which is
 * usually a cache line size) so three mutexes actually give better performance
 * if editing e.g score and ball at the same time by two threads. This is not
 * required but it enhances efficiency of this structure
 *
 * In arrays, index[0] is you and [1] is your enemy
 * for example, pos[0] is your pos and pos[1] is your enemy pos
 */
typedef struct {
  mtx_t mtx;

  float player_y;
  float opponent_y;
  Ball ball;
  unsigned int score[2];

  atomic_bool running;
} SharedData;
