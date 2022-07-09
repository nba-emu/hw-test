#include <gba_base.h>
#include <gba_dma.h>
#include <gba_video.h>

IWRAM_CODE int main(void) {
  REG_DISPCNT = MODE_3 | BG2_ON;

  // initialize affine transform so that every scanline
  // is fetched from the start of VRAM
  REG_BG2PA = 0x100;
  REG_BG2PB = 0;
  REG_BG2PC = 0;
  REG_BG2PD = 0;

  // initialize the bitmap.
  for (int x = 0; x < 240; x++) {
    ((u16*)0x06000000)[x] = x == 239 ? 0x7FFF : 0x0668;
  }

  // initialise backdrop color
  *((u16*)0x05000000) = 0x0668;

  u16 vcount_old = REG_VCOUNT;

  u16 src_buffer[240];

  for (int i = 0; i < 240; i++) {
    src_buffer[i] = i << 8;
  }

  while (true) {
    // sync to start of the next scanline
    while (REG_VCOUNT == vcount_old);
    vcount_old = REG_VCOUNT;

    REG_DMA0SAD = (u32)src_buffer;
    REG_DMA0DAD = (u32)&REG_BG2X;
    REG_DMA0CNT = DMA_ENABLE | DMA_DST_FIXED | DMA_SRC_INC | DMA16 | DMA_HBLANK | 240;
    while (REG_DMA0CNT & DMA_ENABLE);
  }
}
