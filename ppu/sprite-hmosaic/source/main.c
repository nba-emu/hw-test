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

  REG_DISPCNT = OBJ_ENABLE | OBJ_1D_MAP | 3;

  make_obj_palette();
  make_sprite_tile();

  for(int i = 0; i < 16; i++) {
    OAM[i*8+0].attr0 = OBJ_Y(1 + i * 10);
    OAM[i*8+0].attr1 = OBJ_X(8) | OBJ_SIZE(0);
    OAM[i*8+0].attr2 = OBJ_CHAR(512);

    OAM[i*8+1].attr0 = OBJ_Y(1 + i * 10) | OBJ_MOSAIC;
    OAM[i*8+1].attr1 = OBJ_X(16) | OBJ_SIZE(0);
    OAM[i*8+1].attr2 = OBJ_CHAR(512);

    OAM[i*8+2].attr0 = OBJ_Y(1 + i * 10) | OBJ_MOSAIC;
    OAM[i*8+2].attr1 = OBJ_X(24) | OBJ_SIZE(0);
    OAM[i*8+2].attr2 = OBJ_CHAR(512);

    OAM[i*8+3].attr0 = OBJ_Y(1 + i * 10);
    OAM[i*8+3].attr1 = OBJ_X(32) | OBJ_SIZE(0);
    OAM[i*8+3].attr2 = OBJ_CHAR(512);

// --------

    OAM[i*8+4].attr0 = OBJ_Y(1 + i * 10) | OBJ_MOSAIC;
    OAM[i*8+4].attr1 = OBJ_X(40) | OBJ_SIZE(0);
    OAM[i*8+4].attr2 = OBJ_CHAR(512) | OBJ_PRIORITY(2);

    OAM[i*8+5].attr0 = OBJ_Y(1 + i * 10) | OBJ_MOSAIC;
    OAM[i*8+5].attr1 = OBJ_X(48) | OBJ_SIZE(0);
    OAM[i*8+5].attr2 = OBJ_CHAR(512) | OBJ_PRIORITY(0);

    OAM[i*8+6].attr0 = OBJ_Y(1 + i * 10) | OBJ_MOSAIC;
    OAM[i*8+6].attr1 = OBJ_X(56) | OBJ_SIZE(0);
    OAM[i*8+6].attr2 = OBJ_CHAR(512) | OBJ_PRIORITY(0);

    OAM[i*8+7].attr0 = OBJ_Y(1 + i * 10) | OBJ_MOSAIC;
    OAM[i*8+7].attr1 = OBJ_X(64) | OBJ_SIZE(0);
    OAM[i*8+7].attr2 = OBJ_CHAR(512) | OBJ_PRIORITY(1);
  }

/*
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
*/

  while(true) {
    asm volatile("nop\n"); // infinite loops are not UB if we add NOPs to them, right? :o
    VBlankIntrWait();
  }
}