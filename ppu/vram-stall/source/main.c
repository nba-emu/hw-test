#include <gba_base.h>
#include <gba_console.h>
#include <gba_dma.h>
#include <gba_interrupt.h>
#include <gba_timers.h>
#include <gba_video.h>
#include <stdio.h>

#include "emit.h"
#include "ui.h"

static void console_init() {
  consoleDemoInit();

  //set black background color
  BG_COLORS[0] = RGB8(0, 0, 0);
}

static void sync(int line) {
  int vcount_a;
  int vcount_b;

  if (line == 0) {
    vcount_a = 226;
    vcount_b = 227;
  } else if (line == 1) {
    vcount_a = 227;
    vcount_b = 0;
  } else {
    vcount_a = line - 2;
    vcount_b = line - 1;
  }

  while (REG_VCOUNT != vcount_a);
  while (REG_VCOUNT != vcount_b);
}

static void __test_bg_vram_accesses(int line) {
  u16 results[1232];

  emit_fn wait = emit_get_wait();

  for (int cycle = 0; cycle < 1232; cycle++) {
    // Make sure no IRQ is requested or handled,
    // while we prepare for a H-blank IRQ.
    REG_IME = 0;
    REG_DISPSTAT &= ~LCDC_HBL;

    // Register a H-blank IRQ handler with a delay of 'cycle' cycles.
    irqSet(IRQ_HBLANK, emit_get_test(cycle));

    // Reset timer TM0
    REG_TM0CNT = 0;

    // Sync to the start of the scanline, before the one we're targeting.
    sync(line);
  
    // Acknowledge all pending IRQs and allow the next H-blank IRQ to happen.
    REG_IE = IRQ_HBLANK;
    REG_IF = 0xFFFF;
    REG_IME = 1;
    REG_DISPSTAT |= LCDC_HBL;

    /* Run a sequence of 1024 nops in IWRAM,
     * this will allow the IRQ to be taken immediate,
     * once the CPU registers it.
     */
    wait();

    // Wait for the IRQ handler to complete execution.
    while (REG_IE != 0) ;

    results[cycle] = REG_TM0CNT_L;
  }

  // TODO: make it redundant to reinitialize the console.
  console_init();

  u8* sram = (u8*)0x0E000000;

  int calibration = (int)results[0];

  for (int cycle = 1; cycle < 1232; cycle++) {
    int stalls = results[cycle] - calibration - cycle;

    u8 bit = stalls > 0 ? 1 : 0;

    sram[cycle - 1] = bit;
  }

  ui_view_bitmap(sram, 1231);
}

static void test_mode0_accesses() {
  struct TestData {
    u16 dispcnt;
    u16 bgcnt;
  };

  const UIMenuOption options[] = {
    { "BG0 (4bpp)", NULL },
    { "BG1 (4bpp)", NULL },
    { "BG2 (4bpp)", NULL },
    { "BG3 (4bpp)", NULL },
    { "BG0 (8bpp)", NULL },
    { "BG1 (8bpp)", NULL },
    { "BG2 (8bpp)", NULL },
    { "BG3 (8bpp)", NULL },
  };

  const struct TestData tests[] = {
    { MODE_0 | BG0_ENABLE, BG_16_COLOR },
    { MODE_0 | BG1_ENABLE, BG_16_COLOR },
    { MODE_0 | BG2_ENABLE, BG_16_COLOR },
    { MODE_0 | BG3_ENABLE, BG_16_COLOR },
    { MODE_0 | BG0_ENABLE, BG_256_COLOR },
    { MODE_0 | BG1_ENABLE, BG_256_COLOR },
    { MODE_0 | BG2_ENABLE, BG_256_COLOR },
    { MODE_0 | BG3_ENABLE, BG_256_COLOR },
  };

  int option = ui_show_menu(options, sizeof(options) / sizeof(UIMenuOption), true);

  if (option != -1) {
    struct TestData const* test = &tests[option];

    REG_DISPCNT = test->dispcnt;
    REG_BG0CNT = test->bgcnt;
    REG_BG1CNT = test->bgcnt;
    REG_BG2CNT = test->bgcnt;
    REG_BG3CNT = test->bgcnt;

    __test_bg_vram_accesses(1);
  }
}

static void test_mode2_accesses() {
  const UIMenuOption options[] = {
    { "BG2", NULL },
    { "BG3", NULL },
    { "BG2 + BG3", NULL }
  };

  const u16 dispcnt[] = {
    MODE_2 | BG2_ENABLE,
    MODE_2 | BG3_ENABLE,
    MODE_2 | BG2_ENABLE | BG3_ENABLE
  };

  int option = ui_show_menu(options, sizeof(options) / sizeof(UIMenuOption), true);

  if (option != -1) {
    REG_DISPCNT = dispcnt[option];
    __test_bg_vram_accesses(1);
  }
}

static void test_mode3_accesses() {
  REG_DISPCNT = MODE_3 | BG2_ENABLE;
  __test_bg_vram_accesses(1);
}

static void test_mode4_accesses() {
  REG_DISPCNT = MODE_4 | BG2_ENABLE;
  __test_bg_vram_accesses(1);
}

static void test_mode5_accesses() {
  REG_DISPCNT = MODE_5 | BG2_ENABLE;
  __test_bg_vram_accesses(1);
}

int main(void) {
  irqInit();

  emit_init();
  console_init();

  while (true) {
    UIMenuOption options[] = {
      { "Mode 0", &test_mode0_accesses },
      { "Mode 2", &test_mode2_accesses },
      { "Mode 3", &test_mode3_accesses },
      { "Mode 4", &test_mode4_accesses },
      { "Mode 5", &test_mode5_accesses }
    };

    ui_show_menu(options, sizeof(options) / sizeof(UIMenuOption), false);
  }
}
