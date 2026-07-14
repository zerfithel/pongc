#include <stdint.h>
#include <time.h>

#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

#include "random.h"

Seed xorshift32(Seed rng) {
  Seed x = rng;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rng = x;
  return x;
}

Seed make_seed(void) {
  Seed seed = (Seed)time(NULL);
  seed ^= (Seed)clock();
  seed ^= (Seed)(uintptr_t)&seed;
  seed ^= (Seed)getpid();

  // mix bits
  seed ^= seed >> 16;
  seed *= 0x7feb352d;
  seed ^= seed >> 15;
  seed *= 0x846ca68b;
  seed ^= seed >> 16;

  return seed;
}

float rand_range(float min, float max, Seed *r) {
  Seed x = xorshift32(*r);
  float t = (float)x / (float)UINT32_MAX;
  *r = x;
  return min + t * (max - min);
}
