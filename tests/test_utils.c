/*
 * tests: utils.c
 */

#include <math.h>
#include <stdbool.h>

#include "unity.h"

#include "../src/utils.h"

#define FEQ(a, b)                                                              \
  TEST_ASSERT_TRUE_MESSAGE(fabsf(a - b) < 1e-6f, "float mismatch")

void setUp(void) {}
void tearDown(void) {}

void test_skip_spaces(void) {
  TEST_ASSERT_EQUAL_STRING("hello", skip_spaces("   hello"));
  TEST_ASSERT_EQUAL_STRING("hello", skip_spaces("\t\n hello"));
  TEST_ASSERT_EQUAL_STRING("", skip_spaces("   "));
  TEST_ASSERT_EQUAL_STRING("", skip_spaces(""));
}

void test_valid_ipv4(void) {
  TEST_ASSERT_TRUE(valid_ipv4("0.0.0.0"));
  TEST_ASSERT_TRUE(valid_ipv4("127.0.0.1"));
  TEST_ASSERT_TRUE(valid_ipv4("255.255.255.255"));

  TEST_ASSERT_FALSE(valid_ipv4(NULL));
  TEST_ASSERT_FALSE(valid_ipv4(""));
  TEST_ASSERT_FALSE(valid_ipv4("172.012.0.1"));
  TEST_ASSERT_FALSE(valid_ipv4("256.1.1.1"));
  TEST_ASSERT_FALSE(valid_ipv4("1.1.1"));
  TEST_ASSERT_FALSE(valid_ipv4("1..1.1"));
  TEST_ASSERT_FALSE(valid_ipv4("a.b.c.d"));
  TEST_ASSERT_FALSE(valid_ipv4("..."));
}

void test_valid_port(void) {
  TEST_ASSERT_TRUE(valid_port(1));
  TEST_ASSERT_TRUE(valid_port(80));
  TEST_ASSERT_TRUE(valid_port(65535));

  TEST_ASSERT_FALSE(valid_port(0));
  TEST_ASSERT_FALSE(valid_port(65536));
  TEST_ASSERT_FALSE(valid_port(-1));
}

void test_clamp(void) {
  FEQ(0.0f, clamp(-10, 0, 10));
  FEQ(10.0f, clamp(20, 0, 10));
  FEQ(5.0f, clamp(5, 0, 10));
  FEQ(-5.0f, clamp(-5, -10, 0));
}

void test_normalize2f(void) {
  float x = 3, y = 4;
  normalize2f(&x, &y);
  FEQ(1.0f, sqrtf(x * x + y * y));
  FEQ(0.6f, x);
  FEQ(0.8f, y);

  x = 0;
  y = 0;
  normalize2f(&x, &y);
  FEQ(0.0f, x);
  FEQ(0.0f, y);
}

void test_float_equal(void) {
  TEST_ASSERT_TRUE(float_equal(1.0f, 1.0f));
  TEST_ASSERT_TRUE(float_equal(1.0f, 1.0f + 1e-7f));
  TEST_ASSERT_FALSE(float_equal(1.0f, 1.01f));
  TEST_ASSERT_TRUE(float_equal(0.0f, 1e-7f));
}

void test_ortho(void) {
  float m[16];
  ortho(m, 0, 2, 0, 4);

  FEQ(1.0f, m[0]);
  FEQ(0.5f, m[5]);
  FEQ(-1.0f, m[10]);
  FEQ(-1.0f, m[12]);
  FEQ(-1.0f, m[13]);
  FEQ(1.0f, m[15]);

  for (int i = 0; i < 16; i++) {
    if (i != 0 && i != 5 && i != 10 && i != 12 && i != 13 && i != 15)
      FEQ(0.0f, m[i]);
  }
}

void test_integration(void) {
  TEST_ASSERT_TRUE(valid_ipv4("192.168.1.1"));

  float v = clamp(-1, 0, 10);
  FEQ(0.0f, v);

  float x = 3, y = 4;
  normalize2f(&x, &y);
  FEQ(1.0f, sqrtf(x * x + y * y));
}
