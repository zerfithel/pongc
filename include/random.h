static inline float rand_range(float min, float max) {
  return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}
