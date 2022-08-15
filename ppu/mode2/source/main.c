#include <gba_base.h>
#include <gba_dma.h>
#include <gba_video.h>

// One DMA transfer equals two system cycles
#define CYCLE_TO_INDEX(cycle) ((cycle) >> 1)

// In Mode4 one pixel is fetched every four cycles
#define PIXELS_TO_CYCLES(pixels) ((pixels) * 2)

#define TRANSFER_COUNT CYCLE_TO_INDEX(1232)

IWRAM_DATA u16 dma_buffer[TRANSFER_COUNT];

int main(void) {
  REG_DISPCNT = MODE_2 | BG2_ON;
  REG_BG2CNT = BG_TILE_BASE(0) | BG_MAP_BASE(1) | BG_PRIORITY(1);
  REG_BG3CNT = BG_TILE_BASE(0) | BG_MAP_BASE(1) | CHAR_PALETTE(1);

  // initialise palette
  ((u16*)0x05000000)[0] = 0x7FFF; // White
  ((u16*)0x05000000)[1] = 0x001F; // Red
  ((u16*)0x05000000)[2] = 0x0000; // Black
  ((u16*)0x05000000)[3] = 0x7C00; // Blue
  ((u16*)0x05000000)[4] = 0x07E0; // Green

  // initialise first map entry to point to tile at 0x06000000
  *(u16*)0x06000800 = 0;

  /* PPU begins to fetch the BG2 bitmap 32 cycles into the scanline.
   * It fetches a single pixel from the bitmap every four cycles.
   * Note that we add 226 because our DMA starts in H-blank and not
   * and the beginning of the scanline.
   */
  for (int i = 0; i < TRANSFER_COUNT; i++) {
    if (i < CYCLE_TO_INDEX(226 + 32)) {
      // H-blank and first 32 cycles of H-draw
      dma_buffer[i] = 0; // White
    } else if (i < CYCLE_TO_INDEX(226 + 32 + PIXELS_TO_CYCLES(120))) {
      // x = 0 ... 119
      dma_buffer[i] = 1; // Red
    } else if (i < CYCLE_TO_INDEX(226 + 32 + PIXELS_TO_CYCLES(180))) {
      // x = 120 ... 179
      dma_buffer[i] = 2; // Black
    } else if (i < CYCLE_TO_INDEX(226 + 32 + PIXELS_TO_CYCLES(240))) {
      // x = 180 ... 239
      dma_buffer[i] = 3; // Blue
    } else {
      // Red/Black alternating pattern
      dma_buffer[i] = ((i - (226 + 32)) & 15) < 8 ? 2 : 1;
    }
  }

  u16 vcount_old = REG_VCOUNT;

  // let PPU always fetch from the start of VRAM
  REG_BG2PA = 0;
  REG_BG2PB = 0;
  REG_BG2PC = 0;
  REG_BG2PD = 0;
  REG_BG3PA = 0;
  REG_BG3PB = 0;
  REG_BG3PC = 0;
  REG_BG3PD = 0;

  while (true) {
    while (REG_VCOUNT == vcount_old);

    vcount_old = REG_VCOUNT;

    //if (vcount_old == 78 || vcount_old == 160) {
    //  REG_DISPCNT ^= BG3_ON;
    //}
    
    if (vcount_old >= 106) {
      REG_DISPCNT = MODE_2 | BG2_ON | BG3_ON;
    } else if (vcount_old >= 53) {
      REG_DISPCNT = MODE_2 | BG3_ON;
    } else {
      REG_DISPCNT = MODE_2 | BG2_ON;
    }

    REG_DMA0SAD = (u32)dma_buffer;
    REG_DMA0DAD = 0x06000000;
    REG_DMA0CNT = DMA_ENABLE | DMA_DST_FIXED | DMA_SRC_INC | DMA16 | DMA_HBLANK | TRANSFER_COUNT;
    while (REG_DMA0CNT & DMA_ENABLE);
    *(u16*)0x06000000 = 2; // black on next line
  }
}
