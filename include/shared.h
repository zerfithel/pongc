#pragma once
#include <stdatomic.h>
#include <threads.h>
#include "ball.h"

// Shared data between threads
typedef struct {
  mtx_t players_mtx;
  mtx_t ball_mtx;
  mtx_t score_mtx;
  float        y[2];     // y[0] = my pos, y[1] = his pos
  unsigned int score[2];
  Ball         ball;
  atomic_bool  running;
} SharedData;
