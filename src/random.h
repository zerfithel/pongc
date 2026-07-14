#pragma once

#include <stdint.h>

typedef uint32_t Seed;

Seed xorshift32(Seed rng);
Seed make_seed(void);
float rand_range(float min, float max, Seed *r);
