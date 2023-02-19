#include <gba_base.h>
#include <gba_dma.h>
#include <gba_video.h>

IWRAM_CODE int main(void) {
  REG_DISPCNT = MODE_3 | BG2_ON;

  REG_BG2PA = 0x100;
  REG_BG2PB = 0;
  REG_BG2PC = 0;
  REG_BG2PD = 0x100;

  u16* bitmap = (u16*)0x06000000;

  for(int y = 0; y < 160; y++) {
    u16 color = (y & 0x1F) * 0x0421;

    for(int x = 0; x < 240; x++) {
      *bitmap++ = color;
    }
  }

  u16 vcount_old = REG_VCOUNT;

  u16 src_buffer[160];

  // initialize the DMA source buffer with increasing BG2PD values
  for (int i = 0; i < 160; i++) {
    src_buffer[i] = i;
  }

  while (true) {
    // sync to start of the next scanline
    while (REG_VCOUNT == vcount_old);
    vcount_old = REG_VCOUNT;

    // at HBLANK: DMA the source buffer to BG2X at two cycles per entry
    REG_DMA0SAD = (u32)src_buffer;
    REG_DMA0DAD = (u32)&REG_BG2PD;
    REG_DMA0CNT = DMA_ENABLE | DMA_DST_FIXED | DMA_SRC_INC | DMA16 | DMA_HBLANK | 160;
    while (REG_DMA0CNT & DMA_ENABLE);
  }
}
