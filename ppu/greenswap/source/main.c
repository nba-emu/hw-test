#include <gba_base.h>
#include <gba_dma.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_video.h>

IWRAM_DATA u16 greenswap_buf[309];

void hblank() {
  REG_DMA0SAD = (u32)greenswap_buf;
  REG_DMA0DAD = 0x04000002;
  REG_DMA0CNT = DMA_ENABLE | DMA_HBLANK | DMA16 | DMA_SRC_INC | DMA_DST_FIXED | 309;
}

int main(void) {
  irqInit();
  irqEnable(IRQ_VBLANK);

  REG_DISPCNT = MODE_3 | BG2_ON;

  u16* bitmap = (u16*)0x06000000;

  for(int i = 0; i < 12800; i++) {
    *bitmap++ = 0x001F;
    *bitmap++ = 0x03E0;
    *bitmap++ = 0x7C00;
  }

  u16* greenswap = greenswap_buf;

  for(int i = 0; i < 308; i++) {
    *greenswap++ = 1;
  }

  *greenswap++ = 0;

  irqEnable(IRQ_HBLANK);
  irqSet(IRQ_HBLANK, hblank);

  // // Enable greenswap
  // *(vu16*)0x04000002 = 1;

  while(1) {
    VBlankIntrWait();
  }
}
