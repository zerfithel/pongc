#pragma once

// Vector2
typedef struct {
  float x, y;
} Vec2;

void skip_spaces(const char **str);
bool valid_ipv4(const char *str);
bool valid_port(int port);
void normalize(float *x, float *y);
float clamp(float val, float min, float max);
