
#include <gba_console.h>
#include <gba_dma.h>
#include <gba_timers.h>
#include <stdio.h>

#include "test.h"

int IWRAM_CODE test_immediate() {
  u16 result = 0;

  REG_TM0CNT_H = 0;
  REG_TM0CNT_L = 0;
  REG_TM0CNT_H = TIMER_START;

  REG_DMA0SAD = (u32)&REG_TM0CNT_L;
  REG_DMA0DAD = (u32)&result;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_IMMEDIATE | 1;
  while (REG_DMA0CNT & DMA_ENABLE);

  test_expect("IMM", 20, result);
}

int IWRAM_CODE main(void) {
  consoleDemoInit();

  test_immediate();
  test_print_metrics();

  while (1) {
  }
}


