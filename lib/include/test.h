
#ifndef _TEST_H_
#define _TEST_H_

#include <gba_types.h>

void test_reset();

void test_expect_range(
  const char* name,
  u32 expected_lo,
  u32 expected_hi,
  u32 actual
);

void test_expect(
  const char* name,
  u32 expected,
  u32 actual
);

void test_expect_hex(
  const char* name,
  u32 expected,
  u32 actual
);

void test_print_metrics();

// @todo: this literally has no good reason to exist here.
void IWRAM_CpuSet(const void* source, void* dest, u32 mode);

#endif // _TEST_H_