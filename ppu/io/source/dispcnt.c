#include <gba_console.h>
#include <gba_interrupt.h>
#include <gba_video.h>
#include <gba_systemcalls.h>

#include "dispcnt.h"

#include "ui.h"

// @todo: move this into some kind of util.c
void IWRAM_CODE __attribute__((naked)) nop_sled(int length) {
  #define NOP "nop\n"
  #define NOP8  NOP NOP NOP NOP NOP NOP NOP NOP
  #define NOP64 NOP8 NOP8 NOP8 NOP8 NOP8 NOP8 NOP8 NOP8
  #define NOP512 NOP64 NOP64 NOP64 NOP64 NOP64 NOP64 NOP64 NOP64

  asm(
    "eor r0, #0x0FF\n"
    "eor r0, #0x100\n"
    "add pc, r0, lsl #2\n"
    NOP512
    "bx lr\n"
  );

  #undef NOP
  #undef NOP8
  #undef NOP64
  #undef NOP512
}

void IWRAM_CODE __test_mode_hblank() {
  int line = REG_VCOUNT + 1;

  if(line == 228) line = 0;

  if(line >= 80) {
    REG_DISPCNT = MODE_3 | BG2_ON;

    nop_sled(100 + (line - 80)*2);

    REG_DISPCNT = MODE_0 | BG2_ON;
  } else {
    int line_mod_8 = line & 7;

    if(line_mod_8 == 0) {
      REG_DISPCNT = MODE_3 | BG2_ON;
    }
    if(line_mod_8 == 4) {
      REG_DISPCNT = MODE_4 | BG2_ON;
    }
  }
}

void test_mode() {
  // initialize VRAM with a basic bitmap
  for(int i = 0; i < 240 * 160; i++) {
    ((u16*)0x06000000)[i] = 0x001F; // red (in Mode3)
  }

  u16 lfsr = 0x4000;
  for(int i = 0; i < 30*20; i++) {
    ((u16*)0x06000000)[i] = lfsr & 0x3FF;

    int carry = lfsr & 1;
    lfsr >>= 1;
    if(carry) {
      lfsr ^= 0x6000;
    }
  }

  // setup palette for Mode4
  ((u16*)0x05000000)[ 0] = 0x7C00; // green
  ((u16*)0x05000000)[31] = 0x03E0; // blue

  REG_DISPCNT = MODE_3 | BG2_ON;
  REG_DISPSTAT |= LCDC_HBL;

  irqSet(IRQ_HBLANK, &__test_mode_hblank);
  irqEnable(IRQ_HBLANK);

  while(true) {
    VBlankIntrWait();
  }

  // return to a sane state
  REG_DISPSTAT &= ~LCDC_HBL;
  irqDisable(IRQ_HBLANK);
  consoleDemoInit();
}

void test_dispcnt() {
  UIMenuOption options[] = {
    { "Mode", &test_mode }
  };

  ui_show_menu(options, sizeof(options) / sizeof(UIMenuOption), true);
}