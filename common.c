static int test_count = 0;
static int test_pass_count = 0;

IWRAM_CODE void expect_range(
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
    printf("FAIL %ld\n  expected: %ld", actual, expected_lo);
  }

  test_count++;
}

IWRAM_CODE void expect(
  const char* name,
  u32 expected,
  u32 actual
) {
  expect_range(name, expected, expected, actual);
}

IWRAM_CODE void print_metrics() {
  if (test_pass_count == test_count) {
    puts("\ncongratulations!");
  } else {
    printf("\npass:  %d\ntotal: %d\n", test_pass_count, test_count);
  }
}
