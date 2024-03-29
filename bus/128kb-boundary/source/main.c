
#include <gba_console.h>
#include <gba_dma.h>
#include <gba_timers.h>
#include <stdio.h>

#include "test.h"

typedef void (*Fn)(u32);

IWRAM_CODE void __attribute__((naked)) __attribute__((noinline)) ldm(u32 address) {
  asm(
    "ldmia r0, {r0, r1, r2, r3}\n"
    "bx lr\n"
  );
}

IWRAM_CODE void __attribute__((noinline)) dma_inc(u32 address) {
  u32 dst;

  REG_DMA1SAD = address;
  REG_DMA1DAD = (u32)&dst;
  REG_DMA1CNT = DMA_ENABLE | DMA32 | DMA_SRC_INC | DMA_DST_FIXED | 4;

  asm(
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
  );
}

IWRAM_CODE void __attribute__((noinline)) dma_dec(u32 address) {
  u32 dst;

  REG_DMA1SAD = address + 16;
  REG_DMA1DAD = (u32)&dst;
  REG_DMA1CNT = DMA_ENABLE | DMA32 | DMA_SRC_DEC | DMA_DST_FIXED | 4;

  asm(
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
  );
}


IWRAM_CODE u16 __attribute__((noinline)) time(Fn fn, u32 address) { 
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;

  fn(address);

  return REG_TM0CNT_L;
}

IWRAM_CODE int main(void) {
  consoleDemoInit();

  u16 ldm_07FFFFF8 = time(ldm, 0x07FFFFF8);
  u16 ldm_080003F8 = time(ldm, 0x080003F8);
  u16 ldm_0801FFF0 = time(ldm, 0x0801FFF0);
  u16 ldm_0801FFF8 = time(ldm, 0x0801FFF8);
  u16 ldm_0803FFF0 = time(ldm, 0x0803FFF0);
  u16 ldm_0803FFF8 = time(ldm, 0x0803FFF8);

  u16 dma_inc_07FFFFF8 = time(dma_inc, 0x07FFFFF8);
  u16 dma_inc_080003F8 = time(dma_inc, 0x080003F8);
  u16 dma_inc_0801FFF0 = time(dma_inc, 0x0801FFF0);
  u16 dma_inc_0801FFF8 = time(dma_inc, 0x0801FFF8);
  u16 dma_inc_0803FFF0 = time(dma_inc, 0x0803FFF0);
  u16 dma_inc_0803FFF8 = time(dma_inc, 0x0803FFF8);

  u16 dma_dec_07FFFFF8 = time(dma_dec, 0x07FFFFF8);
  u16 dma_dec_080003F8 = time(dma_dec, 0x080003F8);
  u16 dma_dec_0801FFF0 = time(dma_dec, 0x0801FFF0);
  u16 dma_dec_0801FFF8 = time(dma_dec, 0x0801FFF8);
  u16 dma_dec_0803FFF0 = time(dma_dec, 0x0803FFF0);
  u16 dma_dec_0803FFF8 = time(dma_dec, 0x0803FFF8);

  test_expect("LDM 07FFFFF8", 28, ldm_07FFFFF8);
  test_expect("LDM 080003F8", 38, ldm_080003F8);
  test_expect("LDM 0801FFF0", 38, ldm_0801FFF0);
  test_expect("LDM 0801FFF8", 40, ldm_0801FFF8);
  test_expect("LDM 0803FFF0", 38, ldm_0803FFF0);
  test_expect("LDM 0803FFF8", 40, ldm_0803FFF8);

  test_expect("DMA INC 07FFFFF8", 57, dma_inc_07FFFFF8);
  test_expect("DMA INC 080003F8", 67, dma_inc_080003F8);
  test_expect("DMA INC 0801FFF0", 67, dma_inc_0801FFF0);
  test_expect("DMA INC 0801FFF8", 69, dma_inc_0801FFF8);
  test_expect("DMA INC 0803FFF0", 67, dma_inc_0803FFF0);
  test_expect("DMA INC 0803FFF8", 69, dma_inc_0803FFF8);

  test_expect("DMA DEC 07FFFFF8", 63, dma_dec_07FFFFF8);
  test_expect("DMA DEC 080003F8", 68, dma_dec_080003F8);
  test_expect("DMA DEC 0801FFF0", 70, dma_dec_0801FFF0);
  test_expect("DMA DEC 0801FFF8", 68, dma_dec_0801FFF8);
  test_expect("DMA DEC 0803FFF0", 70, dma_dec_0803FFF0);
  test_expect("DMA DEC 0803FFF8", 68, dma_dec_0803FFF8);

  // Sorry, not enough space :(
  // test_print_metrics();

  while (1) {
  }
}


