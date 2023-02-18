#include <gba_base.h>
#include <gba_console.h>
#include <gba_dma.h>
#include <gba_interrupt.h>
#include <gba_input.h>
#include <gba_timers.h>
#include <gba_video.h>
#include <gba_sprites.h>
#include <stdio.h>

#include "emit.h"
#include "ui.h"

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

static u16 __test_single_access(int line, u32 address, int cycle, emit_fn wait) {
  // Make sure no IRQ is requested or handled,
  // while we prepare for a H-blank IRQ.
  REG_IME = 0;
  REG_DISPSTAT &= ~LCDC_HBL;

  // Register a H-blank IRQ handler with a delay of 'cycle' cycles.
  irqSet(IRQ_HBLANK, emit_get_test(cycle, address));

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

  return REG_TM0CNT_L;
}

static void __test_accesses(int line, u32 address) {
  u16 results[1232];

  emit_fn wait = emit_get_wait();

  for (int cycle = 0; cycle < 1232; cycle++) {
    results[cycle] = __test_single_access(line, address, cycle, wait);
  }

  // TODO: make it redundant to reinitialize the console.
  ui_init();

  u8* sram = (u8*)0x0E000000;

  int calibration = (int)results[0];

  // @todo: this needs a proper solution
  if (results[0] == results[1]) calibration--; // we had a stall on the first sample

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
    u16 bghofs;
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
    { "BG0 (8bpp) (BGHOFS=1)", NULL },
    { "BG0 (8bpp) (BGHOFS=2)", NULL },
    { "BG0 (8bpp) (BGHOFS=3)", NULL },
    { "BG0 (8bpp) (BGHOFS=4)", NULL },
    { "BG0 (8bpp) (BGHOFS=5)", NULL },
    { "BG0 (8bpp) (BGHOFS=6)", NULL },
    { "BG0 (8bpp) (BGHOFS=7)", NULL },
    { "BG0 (8bpp) (BGHOFS=8)", NULL }
  };

  const struct TestData tests[] = {
    { MODE_0 | BG0_ENABLE, BG_16_COLOR, 0 },
    { MODE_0 | BG1_ENABLE, BG_16_COLOR, 0 },
    { MODE_0 | BG2_ENABLE, BG_16_COLOR, 0 },
    { MODE_0 | BG3_ENABLE, BG_16_COLOR, 0 },
    { MODE_0 | BG0_ENABLE, BG_256_COLOR, 0 },
    { MODE_0 | BG1_ENABLE, BG_256_COLOR, 0 },
    { MODE_0 | BG2_ENABLE, BG_256_COLOR, 0 },
    { MODE_0 | BG3_ENABLE, BG_256_COLOR, 0 },
    { MODE_0 | BG0_ENABLE, BG_256_COLOR, 1 },
    { MODE_0 | BG0_ENABLE, BG_256_COLOR, 2 },
    { MODE_0 | BG0_ENABLE, BG_256_COLOR, 3 },
    { MODE_0 | BG0_ENABLE, BG_256_COLOR, 4 },
    { MODE_0 | BG0_ENABLE, BG_256_COLOR, 5 },
    { MODE_0 | BG0_ENABLE, BG_256_COLOR, 6 },
    { MODE_0 | BG0_ENABLE, BG_256_COLOR, 7 },
    { MODE_0 | BG0_ENABLE, BG_256_COLOR, 8 }
  };

  int option = ui_show_menu(options, sizeof(options) / sizeof(UIMenuOption), true);

  if (option != -1) {
    struct TestData const* test = &tests[option];

    REG_DISPCNT = test->dispcnt;
    REG_BG0CNT = test->bgcnt;
    REG_BG1CNT = test->bgcnt;
    REG_BG2CNT = test->bgcnt;
    REG_BG3CNT = test->bgcnt;

    REG_BG0HOFS = test->bghofs;

    __test_accesses(1, 0x06000000);
    REG_BG0HOFS = 0; // TODO: fix this properly
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
    __test_accesses(1, 0x06000000);
  }
}

static void test_mode3_accesses() {
  REG_DISPCNT = MODE_3 | BG2_ENABLE;
  __test_accesses(1, 0x06000000);
}

static void test_mode4_accesses() {
  REG_DISPCNT = MODE_4 | BG2_ENABLE;
  __test_accesses(1, 0x06000000);
}

static void test_mode5_accesses() {
  REG_DISPCNT = MODE_5 | BG2_ENABLE;
  __test_accesses(1, 0x06000000);
}

static void test_pram_accesses() {
  UIMenuOption options[] = {
    /*0*/ { "(OFF)", NULL},                        // 1000
    /*1*/ { "(INC   0   0  )", NULL },             // 1000
    /*2*/ { "(INC   BG0 0  )", NULL },             // 1000
    /*3*/ { "(ALPHA 0   0  )", NULL },             // 1000
    /*4*/ { "(ALPHA BG0 0  )", NULL },             // 1000
    /*5*/ { "(ALPHA BG0 BD )", NULL },             // 1000 (1st==transparent) OR 1010 (1st==opaque) 
    /*6*/ { "(ALPHA BG0 BG1) BG0=1 BG1=0", NULL }, // 1000
    /*7*/ { "(ALPHA BG0 BG1) BG0=1 BG1=1", NULL }, // 1000 (1st==transparent || 2nd==transparent) OR 1010 (1st==opaque && 2nd==opaque)
    /*8*/ { "(ALPHA BG0 BG1) BG0=0 BG1=1", NULL }, // 1000
    /*9*/ { "Mode 3", NULL },                      // 0000
   /*10*/ { "Mode 3 + BD blend", NULL }            // 0010
  };

  int option = ui_show_menu(options, sizeof(options) / sizeof(UIMenuOption), true);

  if (option != -1) {
    REG_BG1CNT = REG_BG0CNT;
    REG_DISPCNT = MODE_0 | BG0_ENABLE;

    REG_BLDALPHA = (8 << 8) | 8; // EVA=8, EVB=8
    REG_BLDY = 8;

    switch (option) {
      case 0: {
        REG_BLDCNT = 0;
        break;
      }
      case 1: {
        REG_BLDCNT = (2 << 6); // SFX=inc, 1st=0, 2nd=0
        break;
      }
      case 2: {
        REG_BLDCNT = (2 << 6) | 1; // SFX=inc, 1st=BG0, 2nd=0
        break;
      }
      case 3: {
        REG_BLDCNT = (1 << 6); // SFX=alpha, 1st=0, 2nd=0
        break;
      }
      case 4: {
        REG_BLDCNT = (1 << 6) | 1; // SFX=alpha, 1st=BG0, 2nd=0
        break;
      }
      case 5: {
        REG_BLDCNT = (1 << 6) | 1 | (1 << 13); // SFX=alpha, 1st=BG0, 2nd=backdrop
        break;
      }
      case 6: {
        REG_BLDCNT = (1 << 6) | 1 | (1 << 9);
        break;
      }
      case 7: {
        REG_DISPCNT |= BG1_ENABLE;
        REG_BLDCNT = (1 << 6) | 1 | (1 << 9);
        break;
      }
      case 8: {
        REG_DISPCNT &= ~BG0_ENABLE;
        REG_DISPCNT |=  BG1_ENABLE;
        REG_BLDCNT = (1 << 6) | 1 | (1 << 9);
        break;
      }
      case 9: {
        REG_DISPCNT = MODE_3 | BG2_ENABLE;
        REG_BLDCNT = 0;
        break;
      }
      case 10: {
        REG_DISPCNT = MODE_3 | BG2_ENABLE;
        REG_BLDCNT = (1 << 6) | 4 | (1 << 13); // SFX=alpha, 1st=BG2, 2nd=backdrop
        break;
      }
    }

    __test_accesses(1, 0x05000000);

    REG_BLDCNT = 0;
  }
}

void test_sprite_accesses() {
  UIMenuOption options[] = {
    {"3x 4bpp --- 16px (OAM)", NULL},
    {"3x 4bpp --- 16px (VRAM)", NULL},
    {"3x 8bpp --- 16px (OAM)", NULL},
    {"3x 8bpp --- 16px (VRAM)", NULL},
    {"3x 4bpp RS- 16px (OAM)", NULL},
    {"3x 4bpp RS- 16px (VRAM)", NULL},
    {"3x 4bpp RSD 16px (OAM)", NULL},
    {"3x 4bpp RSD 16px (VRAM)", NULL},
    {"Unlocked H-blank (OAM)", NULL},
    {"Unlocked H-blank (VRAM)", NULL},
    {"Locked H-blank (OAM)", NULL},
    {"Locked H-blank (VRAM)", NULL},
    {"Horizontal clipping (OAM)", NULL},
    {"Horizontal clipping (VRAM)", NULL}
  };

  int option = ui_show_menu(options, sizeof(options) / sizeof(UIMenuOption), true);

  if (option != -1) {
    REG_DISPCNT = MODE_0 | OBJ_ENABLE;

    // initialize OAM with all disabled sprites
    for (int i = 0; i < 128; i++) {
      OAM[i].attr0 = OBJ_DISABLE;
      OAM[i].attr1 = 0;
      OAM[i].attr2 = 0;
    }

    switch (option) {
      case 0: {
        // 3x 4bpp --- 16px (OAM)
        for (int i = 0; i < 3; i++) {
          OAM[i].attr0 = 0; // enable
          OAM[i].attr1 = OBJ_SIZE(1); // 16x16 size
        }
        __test_accesses(1, 0x07000000);
        break;
      }
      case 1: {
        // 3x 4bpp --- 16px (VRAM)
        for (int i = 0; i < 3; i++) {
          OAM[i].attr0 = 0; // enable
          OAM[i].attr1 = OBJ_SIZE(1); // 16x16 size
        }
        __test_accesses(1, 0x06010000);
        break;
      }

      case 2: {
        // 3x 4bpp --- 16px (OAM)
        for (int i = 0; i < 3; i++) {
          OAM[i].attr0 = OBJ_256_COLOR; // enable
          OAM[i].attr1 = OBJ_SIZE(1); // 16x16 size
        }
        __test_accesses(1, 0x07000000);
        break;
      }
      case 3: {
        // 3x 4bpp --- 16px (VRAM)
        for (int i = 0; i < 3; i++) {
          OAM[i].attr0 = OBJ_256_COLOR; // enable
          OAM[i].attr1 = OBJ_SIZE(1); // 16x16 size
        }
        __test_accesses(1, 0x06010000);
        break;
      }

      case 4: {
        // 3x 4bpp RS- 16px (OAM)
        for (int i = 0; i < 3; i++) {
          OAM[i].attr0 = OBJ_ROT_SCALE_ON; // enable
          OAM[i].attr1 = OBJ_SIZE(1); // 16x16 size
        }
        __test_accesses(1, 0x07000000);
        break;
      }
      case 5: {
        // 3x 4bpp RS- 16px (VRAM)
        for (int i = 0; i < 3; i++) {
          OAM[i].attr0 = OBJ_ROT_SCALE_ON; // enable
          OAM[i].attr1 = OBJ_SIZE(1); // 16x16 size
        }
        __test_accesses(1, 0x06010000);
        break;
      }

      case 6: {
        // 3x 4bpp RSD 16px (OAM)
        for (int i = 0; i < 3; i++) {
          OAM[i].attr0 = OBJ_ROT_SCALE_ON | OBJ_DOUBLE;
          OAM[i].attr1 = OBJ_SIZE(1); // 16x16 size
        }
        __test_accesses(1, 0x07000000);
        break;
      }
      case 7: {
        // 3x 4bpp RSD 16px (VRAM)
        for (int i = 0; i < 3; i++) {
          OAM[i].attr0 = OBJ_ROT_SCALE_ON | OBJ_DOUBLE;
          OAM[i].attr1 = OBJ_SIZE(1); // 16x16 size
        }
        __test_accesses(1, 0x06010000);
        break;
      }

      case 8: {
        // Unlocked H-blank (OAM)
        REG_DISPCNT |= BIT(5); // enable fast OAM access during H-blank
        for (int i = 0; i < 128; i++) {
          OAM[i].attr0 = 0; // enable
          OAM[i].attr1 = OBJ_SIZE(1); // 16x16 size
        }
        __test_accesses(1, 0x07000000);
        break;
      }
      case 9: {
        // Unlocked H-blank (VRAM)
        REG_DISPCNT |= BIT(5); // enable fast OAM access during H-blank
        for (int i = 0; i < 128; i++) {
          OAM[i].attr0 = 0; // enable
          OAM[i].attr1 = OBJ_SIZE(1); // 16x16 size
        }
        __test_accesses(1, 0x06010000);
        break;
      }
      case 10: {
        // Locked H-blank (OAM)
        for (int i = 0; i < 128; i++) {
          OAM[i].attr0 = 0; // enable
          OAM[i].attr1 = OBJ_SIZE(1); // 16x16 size
        }
        __test_accesses(1, 0x07000000);
        break;
      }
      case 11: {
        // Locked H-blank (VRAM)
        for (int i = 0; i < 128; i++) {
          OAM[i].attr0 = 0; // enable
          OAM[i].attr1 = OBJ_SIZE(1); // 16x16 size
        }
        __test_accesses(1, 0x06010000);
        break;
      }

      case 12: {
        // Horizontal clipping (OAM)

        // Partially offscreen on the left screen border
        OAM[0].attr0 = 0; // enable
        OAM[0].attr1 = OBJ_SIZE(1) | OBJ_X(-7); // 16x16 size, X=-7

        // Fully offscreen
        OAM[1].attr0 = 0; // enable
        OAM[1].attr1 = OBJ_SIZE(1) | OBJ_X(-20); // 16x16 size, X=-20

        // Partially offscreen on the right screen border
        OAM[2].attr0 = 0; // enable
        OAM[2].attr1 = OBJ_SIZE(1) | OBJ_X(232); // 16x16 size, X=232

        __test_accesses(1, 0x07000000);
        break;
      }
      case 13: {
        // Horizontal clipping (VRAM)

        // Partially offscreen on the left screen border
        OAM[0].attr0 = 0; // enable
        OAM[0].attr1 = OBJ_SIZE(1) | OBJ_X(-7); // 16x16 size, X=-7

        // Fully offscreen
        OAM[1].attr0 = 0; // enable
        OAM[1].attr1 = OBJ_SIZE(1) | OBJ_X(-20); // 16x16 size, X=-20

        // Partially offscreen on the right screen border
        OAM[2].attr0 = 0; // enable
        OAM[2].attr1 = OBJ_SIZE(1) | OBJ_X(232); // 16x16 size, X=232

        __test_accesses(1, 0x06010000);
        break;
      }
    }
  }
}

void IWRAM_CODE test_dispcnt_latch_bg_vram_fetch() {
  /**
   * BG VRAM fetch happens only if both the latched and current BG enable bit are set.
   *
   * @todo: test the effect of BGHOFS[0:2] on the timing.
   */
  u16 results[4];

  u16 saved_dispcnt = REG_DISPCNT;

  REG_DISPCNT &= ~BG_ALL_ON;
  REG_BG0HOFS = 7;
  REG_BG1HOFS = 7;
  REG_BG2HOFS = 7;
  REG_BG3HOFS = 7;

  REG_DMA0SAD = 0x06000000;
  REG_DMA0DAD = 0x06000000;

  while(REG_VCOUNT == 2) ;
  while(REG_VCOUNT != 2) ;
  REG_DISPCNT |= BG_ALL_ON;
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE );
  results[0] = REG_TM0CNT_L;

  while(REG_VCOUNT != 3) ; // hopefully we're still @ line 2
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE);
  results[1] = REG_TM0CNT_L;

  while(REG_VCOUNT != 4) ; // hopefully we're still @ line 3
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE);
  results[2] = REG_TM0CNT_L;

  while(REG_VCOUNT != 5) ; // hopefully we're still @ line 4
  REG_DISPCNT &= ~BG_ALL_ON;
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE);
  results[3] = REG_TM0CNT_L;

  REG_TM0CNT_H = 0;
  REG_DISPCNT = saved_dispcnt;
  REG_BG0HOFS = 0;
  REG_BG1HOFS = 0;
  REG_BG2HOFS = 0;
  REG_BG3HOFS = 0;

  ui_clear();
  printf("%d %d %d %d\n", results[0], results[1], results[2], results[3]);

  while(true) {
    scanKeys();
    if(keysUp() & KEY_B) break;
  }
}

void IWRAM_CODE test_dispcnt_latch_bg_vram_fetch_forced_blank() {
  /**
   * BG VRAM fetch happens only if both the latched and current 'forced blank' bits are cleared.
   *
   * @todo: test the effect of BGHOFS[0:2] on the timing.
   */
  u16 results[4];

  u16 saved_dispcnt = REG_DISPCNT;

  REG_DISPCNT |= LCDC_OFF | BG_ALL_ON;

  REG_DMA0SAD = 0x06000000;
  REG_DMA0DAD = 0x06000000;

  while(REG_VCOUNT == 2) ;
  while(REG_VCOUNT != 2) ;
  REG_DISPCNT &= ~LCDC_OFF;
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE );
  results[0] = REG_TM0CNT_L;

  while(REG_VCOUNT != 3) ; // hopefully we're still @ line 2
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE);
  results[1] = REG_TM0CNT_L;

  while(REG_VCOUNT != 4) ; // hopefully we're still @ line 3
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE);
  results[2] = REG_TM0CNT_L;

  while(REG_VCOUNT != 5) ; // hopefully we're still @ line 4
  REG_DISPCNT |= LCDC_OFF;
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE);
  results[3] = REG_TM0CNT_L;

  REG_TM0CNT_H = 0;
  REG_DISPCNT = saved_dispcnt;

  ui_clear();
  printf("%d %d %d %d\n", results[0], results[1], results[2], results[3]);

  while(true) {
    scanKeys();
    if(keysUp() & KEY_B) break;
  }
}

void IWRAM_CODE test_dispcnt_latch_obj_vram_fetch() {
  // OBJ VRAM fetch happens only if the current OBJ enable bit is set (the latched one is disregarded).

  u16 results[4];

  u16 saved_dispcnt = REG_DISPCNT;

  REG_DISPCNT &= ~OBJ_ENABLE;

  for (int i = 0; i < 128; i++) {
    OAM[i].attr0 = 0; // enable
    OAM[i].attr1 = OBJ_SIZE(3); // 64x64 size
  }

  REG_DMA0SAD = 0x06010000;
  REG_DMA0DAD = 0x06010000;

  while(REG_VCOUNT == 2) ;
  while(REG_VCOUNT != 2) ;
  REG_DISPCNT |= OBJ_ENABLE;
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE );
  results[0] = REG_TM0CNT_L;

  while(REG_VCOUNT != 3) ; // hopefully we're still @ line 2
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE);
  results[1] = REG_TM0CNT_L;

  while(REG_VCOUNT != 4) ; // hopefully we're still @ line 3
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE);
  results[2] = REG_TM0CNT_L;

  while(REG_VCOUNT != 5) ; // hopefully we're still @ line 4
  REG_DISPCNT &= ~OBJ_ENABLE;
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE);
  results[3] = REG_TM0CNT_L;

  REG_TM0CNT_H = 0;
  REG_DISPCNT = saved_dispcnt;

  ui_clear();
  printf("%d %d %d %d\n", results[0], results[1], results[2], results[3]);

  while(true) {
    scanKeys();
    if(keysUp() & KEY_B) break;
  }
}

void IWRAM_CODE test_dispcnt_latch_obj_vram_fetch_forced_blank() {
  // OBJ VRAM fetch does not seem to be affected by 'forced blank' at all???

  u16 results[4];

  u16 saved_dispcnt = REG_DISPCNT;

  REG_DISPCNT |= LCDC_OFF | OBJ_ENABLE;

  for (int i = 0; i < 128; i++) {
    OAM[i].attr0 = 0; // enable
    OAM[i].attr1 = OBJ_SIZE(3); // 64x64 size
  }

  REG_DMA0SAD = 0x06010000;
  REG_DMA0DAD = 0x06010000;

  while(REG_VCOUNT == 2) ;
  while(REG_VCOUNT != 2) ;
  REG_DISPCNT &= ~LCDC_OFF;
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE );
  results[0] = REG_TM0CNT_L;

  while(REG_VCOUNT != 3) ; // hopefully we're still @ line 2
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE);
  results[1] = REG_TM0CNT_L;

  while(REG_VCOUNT != 4) ; // hopefully we're still @ line 3
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE);
  results[2] = REG_TM0CNT_L;

  while(REG_VCOUNT != 5) ; // hopefully we're still @ line 4
  REG_DISPCNT |= LCDC_OFF;
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_SRC_FIXED | DMA_DST_FIXED | 128;
  while(REG_DMA0CNT & DMA_ENABLE);
  results[3] = REG_TM0CNT_L;

  REG_TM0CNT_H = 0;
  REG_DISPCNT = saved_dispcnt;

  ui_clear();
  printf("%d %d %d %d\n", results[0], results[1], results[2], results[3]);

  while(true) {
    scanKeys();
    if(keysUp() & KEY_B) break;
  }
}

void test_dispcnt_latch() {
  UIMenuOption options[] = {
    { "BG VRAM fetch", &test_dispcnt_latch_bg_vram_fetch },
    { "BG VRAM fetch (LCDC off)", &test_dispcnt_latch_bg_vram_fetch_forced_blank },
    { "OBJ VRAM fetch", &test_dispcnt_latch_obj_vram_fetch },
    { "OBJ VRAM fetch (LCDC off)", &test_dispcnt_latch_obj_vram_fetch_forced_blank }
  };

  ui_show_menu(options, sizeof(options) / sizeof(UIMenuOption), true);
}

int main(void) {
  irqInit();

  emit_init();
  ui_init();

  while (true) {
    UIMenuOption options[] = {
      { "Mode 0", &test_mode0_accesses },
      { "Mode 2", &test_mode2_accesses },
      { "Mode 3", &test_mode3_accesses },
      { "Mode 4", &test_mode4_accesses },
      { "Mode 5", &test_mode5_accesses },
      { "PRAM", &test_pram_accesses },
      { "Sprite", &test_sprite_accesses },
      { "DISPCNT latch", &test_dispcnt_latch}
    };

    ui_show_menu(options, sizeof(options) / sizeof(UIMenuOption), false);
  }
}
