#include "unity.h"

/*
 * tests: utils.c
 */
void test_skip_spaces(void);
void test_valid_ipv4(void);
void test_valid_port(void);
void test_clamp(void);
void test_normalize2f(void);
void test_float_equal(void);
void test_ortho(void);
void test_integration(void);

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_skip_spaces);
  RUN_TEST(test_valid_ipv4);
  RUN_TEST(test_valid_port);
  RUN_TEST(test_clamp);
  RUN_TEST(test_normalize2f);
  RUN_TEST(test_float_equal);
  RUN_TEST(test_ortho);
  RUN_TEST(test_integration);

  return UNITY_END();
}
