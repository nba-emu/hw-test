
#include <gba_console.h>
#include <gba_dma.h>
#include <gba_timers.h>
#include <stdio.h>

#include "test.h"

/*int IWRAM_CODE test_immediate() {
  u16 result = 0;

  REG_TM0CNT_H = 0;
  REG_TM0CNT_L = 0;
  REG_TM0CNT_H = TIMER_START;

  REG_DMA1SAD = (u32)&REG_TM0CNT_L;
  REG_DMA1DAD = (u32)&result;
  REG_DMA1CNT = DMA_ENABLE | DMA16 | DMA_IMMEDIATE | 1;
  while (REG_DMA1CNT & DMA_ENABLE);

  test_expect("IMM", 20, result);
}*/

void __attribute__((naked)) __attribute__ ((noinline)) __test(u32 address, u32 value) {
  asm(
    "str r1, [r0]\n"
    "mov r0, r0\n"
    "mov r0, r0\n"
    "mov r0, r0\n"
    "mov r0, r0\n"
    "bx lr"
  );
}

void test_ewram() {
  REG_DMA1SAD = 0x02000000;
  REG_DMA1DAD = 0x02000000;

  REG_TM0CNT_H = 0;
  REG_TM0CNT_L = 0;
  REG_TM0CNT_H = TIMER_START;

  __test((u32)&REG_DMA1CNT, DMA_ENABLE | DMA16 | DMA_IMMEDIATE | 1);

  u16 result = REG_TM0CNT_L;

  test_expect("EWRAM DMA", 88, result);
}

void test_rom() {
  u16 tmp;
  REG_DMA1SAD = 0x08000000;
  REG_DMA1DAD = (u32)&tmp;

  REG_TM0CNT_H = 0;
  REG_TM0CNT_L = 0;
  REG_TM0CNT_H = TIMER_START;

  __test((u32)&REG_DMA1CNT, DMA_ENABLE | DMA16 | DMA_IMMEDIATE | 1);

  u16 result = REG_TM0CNT_L;

  test_expect("ROM DMA", 88, result);
}

int main(void) {
  consoleDemoInit();

  test_ewram();
  test_rom();
  test_print_metrics();

  while (1) {
  }
}


