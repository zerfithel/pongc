/*
The MIT License (MIT)

Copyright © 2026 Zerfithel

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

// skip whitespaces
void skip_spaces(const char **str) {
  while (**str != '\0' && isspace(**str)) {
    (*str) += 1;
  }
  return;
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
      ptr  += 1;
      continue;
    }
    num = 0;
    int digits = 0;

    while (*ptr && *ptr != '.') {
      num = num * 10 + (*ptr - '0');
      ptr    += 1;
      digits += 1;
    }

    if (digits == 0 || num > 255) {
      return false;
    }
  }

  return dots == 3;
}

// doesnt include 0 as a port
bool valid_port(int port) {
  if (port > 0 && port <= 65535) {
    return true;
  }
  return false;
}

// normalize 2D vector
void normalize(float *x, float *y) {
  // length = x^2 + y^2
  float length = sqrtf((*x)*(*x) + (*y)*(*y));
  if (length != 0.0f) {
    *x /= length;
    *y /= length;
  }
  return;
}

float clamp(float val, float min, float max) {
  if (val < min) return min;
  if (val > max) return max;
  return val;
}
