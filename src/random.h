#pragma once

#include <time.h>
#include <stdint.h>

#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif // ifdef _WIN32

/*
 * XOR Shift
 */
static inline uint32_t xorshift32(uint32_t rng) {
  uint32_t x = rng;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rng = x;
  return x;
}

/*
 * generate (random enough) seed
 */
static inline uint32_t make_seed(void) {
  uint32_t seed = (uint32_t)time(NULL);
  seed ^= (uint32_t)clock();
  seed ^= (uint32_t)(uintptr_t)&seed;
  seed ^= (uint32_t)getpid();

  // mix bits 
  seed ^= seed >> 16;
  seed *= 0x7feb352d;
  seed ^= seed >> 15;
  seed *= 0x846ca68b;
  seed ^= seed >> 16;

  return seed;
}

/*
 * Function to generate random float in given range (min to max)
 */
static inline float rand_range(float min, float max, uint32_t r) {
  uint32_t x = xorshift32(r);
  float t = (float)x / (float)UINT32_MAX;
  return min + t * (max - min);
}

