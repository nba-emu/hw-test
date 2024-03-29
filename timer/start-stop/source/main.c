#include <gba_base.h>
#include <gba_console.h>
#include <gba_timers.h>
#include <stdio.h>

#include "test.h"

IWRAM_CODE void test_start_stop() {
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;

  asm(
    "nop\n"
    "nop\n"
    "nop\n"
  );

  u16 sample1 = REG_TM0CNT_L;

  asm("nop");

  REG_TM0CNT_H = 0;

  asm(
    "nop\n"
    "nop\n"
    "nop\n"
  );

  u16 sample2 = REG_TM0CNT_L;

  test_expect("1ST", 3, sample1);
  test_expect("2ND", 8, sample2);
}

IWRAM_CODE int main(void) {
  consoleDemoInit();
  
  test_start_stop();
  test_print_metrics();

  while (1) {
  }
}
