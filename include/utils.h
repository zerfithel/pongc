#pragma once

// Vector2
typedef struct {
  float x, y;
} Vec2;

// Skips whitespaces and returns str ptr that points to first character (that isnt whitespace, tab, etc.)
const char *skip_spaces(const char *str);

// Returns:
// true  -> if given string is a valid IPv4 address
// false -> if its not a valid IPv4 address
bool valid_ipv4(const char *str);

// Returns:
// true  -> if given string is a valid IPv4 address
// false -> if its not a valid port
// Does not include 0 as a port
bool valid_port(int port);

// Clamps the given val by returning the value between min and max
// used to clamp player pos to map
float clamp(float val, float min, float max);

// Initializing 2D orthographic projection 4x4 array
void ortho(float *m, float l, float r, float b, float t);

// Normalize 2D vector float (x, y)
void normalize2f(float *x, float *y);
