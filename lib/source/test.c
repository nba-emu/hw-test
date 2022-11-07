#include <gba_systemcalls.h>
#include <stdio.h>

#include "test.h"

static int test_count = 0;
static int test_pass_count = 0;

IWRAM_CODE void test_reset() {
  test_count = 0;
  test_pass_count = 0;
}

IWRAM_CODE void test_expect_range(
  const char* name,
  u32 expected_lo,
  u32 expected_hi,
  u32 actual
) {
  bool pass = actual >= expected_lo && actual <= expected_hi;

  printf("%s: ", name);

  if (pass) {
    printf("PASS %ld\n", actual);
    test_pass_count++;
  } else if (expected_lo != expected_hi) {
    printf("FAIL %ld\n  expected: %ld - %ld\n", actual, expected_lo, expected_hi);
  } else {
    printf("FAIL %ld\n  expected: %ld\n", actual, expected_lo);
  }

  test_count++;
}

IWRAM_CODE void test_expect(
  const char* name,
  u32 expected,
  u32 actual
) {
  test_expect_range(name, expected, expected, actual);
}

IWRAM_CODE void test_expect_hex(
  const char* name,
  u32 expected,
  u32 actual
) {
  printf("%s: ", name);

  if (expected == actual) {
    printf("PASS 0x%lx\n", actual);
    test_pass_count++;
  } else {
    printf("FAIL 0x%lx\n  expected: 0x%lx\n", actual, expected);
  }

  test_count++;
}

IWRAM_CODE void test_print_metrics() {
  if (test_pass_count == test_count) {
    puts("\ncongratulations!");
  } else {
    printf("\npass:  %d\ntotal: %d\n", test_pass_count, test_count);
  }
}

// @todo: move this somewhere else...
IWRAM_CODE void IWRAM_CpuSet(const void* source, void* dest, u32 mode) {
  SystemCall(11);
}
