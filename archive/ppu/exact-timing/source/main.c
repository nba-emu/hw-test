#include <gba_base.h>
#include <gba_console.h>
#include <gba_dma.h>
#include <gba_timers.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <stdio.h>  

#include "test.h"

IWRAM_CODE void test_video_transfer_dma_start() {
  u16 result;

  // sync to the next frame start:
  while (REG_VCOUNT != 227);
  while (REG_VCOUNT != 0);

  // start timer0 at one tick per cycle:
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;

  REG_DMA3SAD = (u32)&REG_TM0CNT_L;
  REG_DMA3DAD = (u32)&result;
  REG_DMA3CNT = DMA_ENABLE | DMA_SRC_FIXED | DMA_DST_FIXED | DMA16 | DMA_SPECIAL | 1;
  while (REG_DMA3CNT & DMA_ENABLE);

  test_expect_hex("DMA3 XFER", 0x52d3, result);
}

IWRAM_CODE void test_hblank_flag_set_unset() {
  vu16 buffer[616];

  REG_DISPSTAT = (REG_DISPSTAT & 0xFF) | (3 << 8);

  // sync to the next frame start:
  while (REG_VCOUNT != 227);
  while (REG_VCOUNT != 0);

  REG_DMA3SAD = (u32)&REG_DISPSTAT;
  REG_DMA3DAD = (u32)buffer;
  REG_DMA3CNT = DMA_ENABLE | DMA_SRC_FIXED | DMA_DST_INC | DMA16 | DMA_SPECIAL | 616;
  while (REG_DMA3CNT & DMA_ENABLE);

  int hblank_set = -1;
  int hblank_unset = -1;

  for (int i = 0; i < 616; i++) {
    if (buffer[i] & LCDC_HBL_FLAG) {
      hblank_set = i;
      break;
    }
  }

  for (int i = hblank_set + 1; i < 616; i++) {
    if (~buffer[i] & LCDC_HBL_FLAG) {
      hblank_unset = i;
      break;
    }
  }

  int vcount_flag_set = -1;

  for (int i = 0; i < 616; i++) {
    if (buffer[i] & LCDC_VCNT_FLAG) {
      vcount_flag_set = i;
      break;
    }
  }

  test_expect("HBL SET", 500, hblank_set);
  test_expect("HBL UNSET", 613, hblank_unset);
  test_expect("VCNT FLAG SET", 613, vcount_flag_set);
}

IWRAM_CODE void test_vcount_change() {
  vu16 buffer[616];

  // sync to the next frame start:
  while (REG_VCOUNT != 227);
  while (REG_VCOUNT != 0);

  REG_DMA3SAD = (u32)&REG_VCOUNT;
  REG_DMA3DAD = (u32)buffer;
  REG_DMA3CNT = DMA_ENABLE | DMA_SRC_FIXED | DMA_DST_INC | DMA16 | DMA_SPECIAL | 616;
  while (REG_DMA3CNT & DMA_ENABLE);

  int vcount_change = -1;

  for (int i = 0; i < 616; i++) {
    // DMA starts on VCOUNT=2 so we look for it to change to 3:
    if (buffer[i] == 3) {
      vcount_change = i;
      break;
    }
  }

  test_expect("VCNT CHANGE", 613, vcount_change);
}

IWRAM_CODE void test_hblank_dma() {
  vu16 buffer[616];

  // sync to the next frame start:
  while (REG_VCOUNT != 227);
  while (REG_VCOUNT != 0);

  // start timer0 at one tick per cycle:
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;

  // setup DMA2 h-blank transfer
  u16 dummy;
  REG_DMA2SAD = (u32)&dummy;
  REG_DMA2DAD = (u32)&dummy;
  REG_DMA2CNT = DMA_ENABLE | DMA16 | DMA_HBLANK | DMA_REPEAT | 1;

  // setup DMA3 video transfer
  REG_DMA3SAD = (u32)&REG_TM0CNT_L;
  REG_DMA3DAD = (u32)buffer;
  REG_DMA3CNT = DMA_ENABLE | DMA_SRC_FIXED | DMA_DST_INC | DMA16 | DMA_SPECIAL | 616;
  while (REG_DMA3CNT & DMA_ENABLE);

  REG_DMA2CNT = 0;

  int hblank_dma = -1;

  for (int i = 1; i < 616; i++) {
    if ((buffer[i] - buffer[i - 1]) != 2) {
      hblank_dma = i;
      break;
    }
  }

  test_expect("HBL DMA", 502, hblank_dma);
}

IWRAM_CODE void test_hblank_irq() {
  vu16 buffer[616];

  // sync to the next frame start:
  while (REG_VCOUNT != 227);
  while (REG_VCOUNT != 0);

  REG_IE = 0;
  REG_DISPSTAT |= LCDC_HBL;

  REG_DMA3SAD = (u32)&REG_IF;
  REG_DMA3DAD = (u32)buffer;
  REG_DMA3CNT = DMA_ENABLE | DMA_SRC_FIXED | DMA_DST_INC | DMA16 | DMA_SPECIAL | 616;

  while (REG_DMA3CNT & DMA_ENABLE) REG_IF = 0xFFFF; // keep acknowleding IRQs until DMA takes the bus over

  int irq_assert = -1;

  for (int i = 0; i < 616; i++) {
    if (buffer[i] & IRQ_HBLANK) {
      irq_assert = i;
      break;
    }
  }

  test_expect("HBL IRQ", 501, irq_assert);
}

IWRAM_CODE int main(void) {
  consoleDemoInit();

  test_video_transfer_dma_start();
  test_hblank_flag_set_unset();
  test_vcount_change();
  test_hblank_dma();
  test_hblank_irq();

  test_print_metrics();

  while (1) {
  }
}
