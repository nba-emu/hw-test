#include <gba.h>
#include <stdio.h>

static void make_obj_palette() {
  *(u16*)0x05000202 = 0x1F;
  *(u16*)0x05000204 = 0x1F << 5;
  *(u16*)0x05000206 = 0x1F << 10;
  *(u16*)0x05000208 = 0x7FFF;
  *(u16*)0x0500020A = 0x03FF;
}

static void make_sprite_tile() {
  u32* tile = (u32*)0x06014000;

  *tile++ = 0x53535353;
  *tile++ = 0x34444445;
  *tile++ = 0x54212143;
  *tile++ = 0x34100245;
  *tile++ = 0x54200143;
  *tile++ = 0x34121245;
  *tile++ = 0x54444443;
  *tile++ = 0x35353535;
}

static void hblank_handler() {
  int line = REG_VCOUNT + 1;
  int mosaic = (line / 10) & 15;

  REG_MOSAIC = mosaic << 8;
}

int main(void) {
  irqInit();
  irqSet(IRQ_HBLANK, hblank_handler);
  irqEnable(IRQ_HBLANK | IRQ_VBLANK);

  REG_DISPCNT = BG2_ENABLE | OBJ_ENABLE | OBJ_1D_MAP | OBJ_WIN_ENABLE | 3;
  REG_WINOUT = 0x04FF;
  REG_BG2CNT = 0;

  make_obj_palette();
  make_sprite_tile();

  for(int i = 0; i < 16; i++) {
    OAM[i*4+0].attr0 = OBJ_Y(1 + i * 10);
    OAM[i*4+0].attr1 = OBJ_X(8) | OBJ_SIZE(0);
    OAM[i*4+0].attr2 = OBJ_CHAR(512);

    OAM[i*4+1].attr0 = OBJ_Y(1 + i * 10) | OBJ_MOSAIC;
    OAM[i*4+1].attr1 = OBJ_X(16) | OBJ_SIZE(0);
    OAM[i*4+1].attr2 = OBJ_CHAR(512);

    OAM[i*4+2].attr0 = OBJ_Y(1 + i * 10) | OBJ_MOSAIC;
    OAM[i*4+2].attr1 = OBJ_X(24) | OBJ_SIZE(0);
    OAM[i*4+2].attr2 = OBJ_CHAR(512);

    OAM[i*4+3].attr0 = OBJ_Y(1 + i * 10);
    OAM[i*4+3].attr1 = OBJ_X(32) | OBJ_SIZE(0);
    OAM[i*4+3].attr2 = OBJ_CHAR(512);
  }

  ((OBJAFFINE*)OAM)[0].pa =  0xB5;
  ((OBJAFFINE*)OAM)[0].pb = -0xB5;
  ((OBJAFFINE*)OAM)[0].pc =  0xB5;
  ((OBJAFFINE*)OAM)[0].pd =  0xB5;

  for(int i = 0; i < 16; i++) {
    OAM[i*2+64].attr0 = OBJ_Y(1 + i * 10 - 3) | ATTR0_ROTSCALE | ATTR0_ROTSCALE_DOUBLE | OBJ_MOSAIC;
    OAM[i*2+64].attr1 = OBJ_X(64) | OBJ_SIZE(0);
    OAM[i*2+64].attr2 = OBJ_CHAR(512);

    OAM[i*2+65].attr0 = OBJ_Y(1 + i * 10 - 3) | ATTR0_ROTSCALE | ATTR0_ROTSCALE_DOUBLE | OBJ_MOSAIC;
    OAM[i*2+65].attr1 = OBJ_X(70) | OBJ_SIZE(0);
    OAM[i*2+65].attr2 = OBJ_CHAR(512);
  }

  while(true) {
    asm volatile("nop\n"); // infinite loops are not UB if we add NOPs to them, right? :o
    VBlankIntrWait();
  }
}