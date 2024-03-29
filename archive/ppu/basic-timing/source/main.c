#include <gba_base.h>
#include <gba_console.h>
#include <gba_dma.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <gba_video.h>
#include <stdio.h> 

// 616 = cycles_per_scanline / cycles_per_sample
#define SAMPLE_COUNT (616)

static u16 samples[SAMPLE_COUNT * 2];

#include "test.h"

// Test when a H-blank DMA is started inside a scanline.
IWRAM_CODE void test_hblank_dma_time() {
  u16 cycle_count = 0;

  REG_TM0CNT_H = 0;
  REG_TM0CNT_L = 0;

  while (REG_VCOUNT != 63) ;
  while (REG_VCOUNT != 64) ;

  REG_TM0CNT_H = TIMER_START;
  REG_DMA0SAD = (u32)&REG_TM0CNT_L;
  REG_DMA0DAD = (u32)&cycle_count;
  REG_DMA0CNT = DMA_DST_FIXED | DMA_SRC_FIXED | DMA_HBLANK | DMA16 | DMA_ENABLE | 1;
  while (REG_DMA0CNT & DMA_ENABLE) ;

  // reads 995 on hardware
  test_expect_range("HBL DMA", 982, 1002, cycle_count);
}

// Test when the H-blank and V-count coincidence bits are set inside a scanline.
IWRAM_CODE void test_hblank_and_vcount_bit() {
  REG_DISPSTAT = 65 << 8;
  while (REG_VCOUNT != 63) ;
  while (REG_VCOUNT != 64) ;

  REG_DMA0SAD = (u32)&REG_DISPSTAT;
  REG_DMA0DAD = (u32)samples;
  REG_DMA0CNT = DMA_DST_INC | DMA_SRC_FIXED | DMA16 | DMA_HBLANK | DMA_ENABLE | (SAMPLE_COUNT * 2);
  while (REG_DMA0CNT & DMA_ENABLE) ;

  // Check H-blank bit set and unset times
  {
    int set = -1;
    int unset = -1;

    for (int i = 0; i < SAMPLE_COUNT * 2; i++) {
      if (samples[i] & 2) {
        set = i;
        break;
      }
    }

    for (int i = set + 1; i < SAMPLE_COUNT * 2; i++) {
      if (~samples[i] & 2) {
        unset = i;
        break;
      }
    }

    test_expect_range("STAT HBL SET", 0, 0, set);
    test_expect_range("STAT HBL UNSET", 111, 111, unset);
  }

  // Check H-blank bit set and unset times
  // TODO: this is almost the same code...
  {
    int set = -1;
    int unset = -1;

    for (int i = 0; i < SAMPLE_COUNT * 2; i++) {
      if (samples[i] & 4) {
        set = i;
        break;
      }
    }

    for (int i = set + 1; i < SAMPLE_COUNT * 2; i++) {
      if (~samples[i] & 4) {
        unset = i;
        break;
      }
    }

    test_expect_range("STAT VCNT SET", 111, 111, set);
    test_expect_range("STAT VCNT UNSET", 727, 727, unset);
  }
}

// Test when a H-blank IRQ is asserted inside a scanline.
IWRAM_CODE void test_hblank_irq() {
  REG_IME = 0;
  REG_DISPSTAT = LCDC_HBL;

  // sync to somewhere at the beginning of a visible scanline.
  while (REG_VCOUNT != 63) ;
  while (REG_VCOUNT != 64) ;

  REG_IF = 0xFFFF;

  REG_DMA0SAD = (u32)&REG_IF;
  REG_DMA0DAD = (u32)samples;
  REG_DMA0CNT = DMA_DST_INC | DMA_SRC_FIXED | DMA16 | DMA_ENABLE | SAMPLE_COUNT;
  while (REG_DMA0CNT & DMA_ENABLE) ;

  int set = -1;

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    if (samples[i] & 2) {
      set = i;
      break;
    }
  }

  // reads 488 on hardware
  test_expect_range("HBL IRQ", 485, 495, set);
}


IWRAM_CODE int main(void) {
  consoleDemoInit();

  test_hblank_dma_time();
  test_hblank_and_vcount_bit();
  test_hblank_irq();
  test_print_metrics();

  while (1) {
  }
}
