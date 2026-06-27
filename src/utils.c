#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

// skip whitespaces
const char *skip_spaces(const char *str) {
  const char *s = str;
  while (*s != '\0' && isspace(*s)) {
    s += 1;
  }
  return s;
}

// check if str is a valid IPv4 address
bool valid_ipv4(const char *str) {
  if (!str) {
    return false;
  }

  int num;
  int dots = 0;
  const char *ptr = str;

  while (*ptr) {
    if (!isdigit(*ptr) && *ptr != '.') {
      return false;
    }
    ptr += 1;
  }
  ptr = str;

  while (*ptr) {
    if (*ptr == '.') {
      dots += 1;
      ptr += 1;
      continue;
    }
    num = 0;
    int digits = 0;

    while (*ptr && *ptr != '.') {
      num = num * 10 + (*ptr - '0');
      ptr += 1;
      digits += 1;
    }

    if (digits == 0 || num > 255) {
      return false;
    }
  }

  return dots == 3;
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
