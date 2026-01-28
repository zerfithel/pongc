#pragma once
#include <stdatomic.h>
#ifdef _WIN32
  #include "../external/tinycthread.h"
#else
  #include <threads.h>
#endif
#include <stdalign.h>  // Aligning bytes in structure to prevent false sharing
#include "ball.h"

// Structure SharedData: Shared data between threads (main, network threads), contains game state and atomic_bool running
//
// To prevent false sharing (cache miss) i added padding (64 bytes which is usually a cache line size) so three mutexes actually give better performance if editing e.g score and ball at the same time by two threads.
// This is not required but it enhances efficiency of this structure
//
// In arrays, index[0] is you and [1] is your enemy
// for example, pos[0] is your pos and pos[1] is your enemy pos

// SharedData
typedef struct { 
  // Mutexes
  alignas(64)      mtx_t        players_mtx;
  alignas(64)      mtx_t        ball_mtx;
  alignas(64)      mtx_t        score_mtx;

  // Game state
  alignas(64)      float        y[2];  
  alignas(64)      Ball         ball; 
  alignas(64)      unsigned int score[2];

  // Is game still running? (Both threads read from this variable to see if they should continue working)
  alignas(64)      atomic_bool  running;
} SharedData;
