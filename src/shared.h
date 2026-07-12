#pragma once

#include <stdalign.h>
#include <stdatomic.h>

#ifdef _WIN32
#include "external/tinycthread.h"
#else
#include <threads.h>
#endif // ifdef _WIN32

#include "ball.h"

// Structure with data shared between threads, before any data write and read
// mtx should be locked and unlocked with an exception for atomic variables
// which should be touched with atomic_* functions
typedef struct {
  mtx_t mtx;

  float player_y;
  float opponent_y;
  Ball ball;
  unsigned int player_score;
  unsigned int opponent_score;

  atomic_bool running;
} SharedData;
