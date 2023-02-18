#include <gba_base.h>
#include <gba_console.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_sprites.h>
#include <gba_video.h>

#include <stdio.h>

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

IWRAM_CODE void hblank() {
  const int line = (REG_VCOUNT + 1) % 228;

  // test 'BG0 enable' bit
  if(line == 0) REG_DISPCNT &= ~BG0_ON;
  if(line == 8) REG_DISPCNT |=  BG0_ON;

  // test 'forced blank' bit
  if(line == 16) REG_DISPCNT |=  LCDC_OFF;
  if(line == 24) REG_DISPCNT &= ~LCDC_OFF;

  // test 'WIN0 enable' bit
  // @todo: probably should simplify this
  if(line == 32) REG_DISPCNT |=  WIN0_ON;
  if(line == 40) REG_DISPCNT &= ~WIN0_ON;
  if(line == 48) REG_DISPCNT |=  WIN0_ON;
  if(line == 56) REG_DISPCNT &= ~WIN0_ON;

  if(line == 64) REG_DISPCNT &= ~OBJ_ON;
  if(line == 72) REG_DISPCNT |=  OBJ_ON;
}

IWRAM_CODE int main(void) {
  consoleDemoInit();

  irqInit();
  irqSet(IRQ_HBLANK, hblank);
  irqEnable(IRQ_HBLANK | IRQ_VBLANK);

  REG_WIN0H = 240;
  REG_WIN0V = 160;
  REG_WININ = 0;
  REG_WINOUT = 1; // display BG0

  puts("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  puts("BCDEFGHIJKLMNOPQRSTUVWXYZA");
  puts("CDEFGHIJKLMNOPQRSTUVWXYZAB");
  puts("DEFGHIJKLMNOPQRSTUVWXYZABC");
  puts("EFGHIJKLMNOPQRSTUVWXYZABCD");
  puts("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  puts("BCDEFGHIJKLMNOPQRSTUVWXYZA");
  puts("CDEFGHIJKLMNOPQRSTUVWXYZAB");
  puts("DEFGHIJKLMNOPQRSTUVWXYZABC");
  puts("EFGHIJKLMNOPQRSTUVWXYZABCD");
  puts("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  puts("BCDEFGHIJKLMNOPQRSTUVWXYZA");
  puts("CDEFGHIJKLMNOPQRSTUVWXYZAB");

  u32* tile = (u32*)0x06010000;

  for(int i = 0; i < 64; i++) {
    *tile++ = 0x12121212;
    *tile++ = 0x21212121;
  }

  *(u16*)0x05000202 = 0x1F;
  *(u16*)0x05000204 = 0x1F << 5;

  OAM[0].attr0 = OBJ_Y(64);
  OAM[0].attr1 = OBJ_SIZE(1);
  OAM[0].attr2 = 0;

  REG_DISPCNT |= OBJ_ENABLE | OBJ_1D_MAP;

  while (true) {
    VBlankIntrWait();
  }
}