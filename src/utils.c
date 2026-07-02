#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "utils.h"

// skip whitespaces
const char *skip_spaces(const char *str) {
  const char *s = str;
  while (*s != '\0' && isspace(*s)) {
    s += 1;
  }
  return s;
}

// check if str is a valid IPv4 address
bool valid_ipv4(const char *ip) {
  if (!ip) {
    return false;
  }

  int dots = 0;
  int consecutive_dots = 0;
  int num = 0;
  int digits_count = 0;

  for (size_t i = 0; ip[i] != '\0'; i++) {
    if (isdigit(ip[i])) {
      num = num * 10 + (ip[i] - '0');
      digits_count++;

      if (digits_count > 1 && num >= 0 && ip[i - 1] == '0') {
        return false;
      }

      if (num > 255) {
        return false;
      }

      consecutive_dots = 0;

    } else if (ip[i] == '.') {
      if (consecutive_dots > 0) {
        return false;
      }

      if (digits_count == 0) {
        return false;
      }

      dots++;
      consecutive_dots++;
      num = 0;
      digits_count = 0;

    } else {
      return false;
    }
  }

  if (dots != 3 || digits_count == 0) {
    return false;
  }

  if (num > 255) {
    return false;
  }

  return 1;
}
// doesnt include 0 as a port
bool valid_port(long port) {
  if (port > 0 && port <= 65535) {
    return true;
  }
  return false;
}

float clamp(float val, float min, float max) {
  if (val < min)
    return min;
  if (val > max)
    return max;
  return val;
}

void ortho(float *m, float l, float r, float b, float t) {
  for (int i = 0; i < 16; i++) {
    m[i] = 0.0f;
  }
  m[0] = 2.0f / (r - l);
  m[5] = 2.0f / (t - b);
  m[10] = -1.0f;
  m[12] = -(r + l) / (r - l);
  m[13] = -(t + b) / (t - b);
  m[15] = 1.0f;
}

void normalize2f(float *x, float *y) {
  // length = sqrtf(x^2 + y^2)
  float length = sqrtf((*x) * (*x) + (*y) * (*y));
  if (length != 0.0f) {
    *x /= length;
    *y /= length;
  }
  return;
}

// compare two floats
bool float_equal(float a, float b) {
  const float EPSILON = 1e-6f;
  return (fabsf(a - b)) < EPSILON;
}
