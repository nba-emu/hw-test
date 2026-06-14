
#include <gba_console.h>
#include <gba_dma.h>
#include <stdio.h>

#include "test.h"

IWRAM_DATA u32 dma_dst_32 = 0;
IWRAM_DATA alignas(4) vu16 dma_dst_1616[2] = {0xFFFF, 0xFFFF};

int main(void) {
  consoleDemoInit();

  // Test that:
  // - DMA latch exists
  // - DMA from addresses < 0x02000000 do not update the latch (and therefore the old latch value is written) (technically we only test 0x00000000)
  // - 16-bit DMAs write the LSW or MSW of the 32-bit latch (depending on alignment).
  //   DMA likely puts the full 32-bit latch on the data bus and the bus extracts the LSW or MSW depending on destination address alignment.
  for(int i = 0; i < 2; i++) {
    static const u16 results[2] = { 0xC0DE, 0xDEAD };
    static const char* const names[2] = { "DMA LATCH 0", "DMA LATCH 1" };

    u32 data;

    // 32-bit DMA1 from IWRAM sets the DMA1 latch to 0xDEADC0DE.
    data = 0xDEADC0DE;
    REG_DMA1SAD = (u32)&data;
    REG_DMA1DAD = (u32)&dma_dst_32;
    REG_DMA1CNT = DMA_ENABLE | DMA32 | 1;
    while ((REG_DMA1CNT) & DMA_ENABLE) ;

    // 16-bit DMA1 from 0x00000000 to dma_dst_1616[0] or dma_dst_1616[1].
    // During the read cycle the DMA1 latch is not updated because the source access is below 0x02000000.
    // During the write cycle, depending on destination address alignment, the DMA1 latch LSW (0xC0DE) or MSW (0xDEAD) is written.
    data = 0; // Reset data in case the old address is used in place of the invalid address
    REG_DMA1SAD = 0x00000000;
    REG_DMA1DAD = (u32)&dma_dst_1616[i];
    REG_DMA1CNT = DMA_ENABLE | DMA16 | 1;
    while ((REG_DMA1CNT) & DMA_ENABLE) ;
  
    test_expect_hex(names[i], results[i], dma_dst_1616[i]);
  }

  // Test that:
  // - Each DMA channel has a separate latch
  // - 16-bit DMAs update the 32-bit latch by duplicating the 16-bit value in the LSW and MSW.
  {
    u32 data;
    
    // 16-bit DMA2 from IWRAM sets the DMA2 latch to 0x12341234 (not 0x????1234 or 0x1234???? !)
    data = 0x1234;
    REG_DMA2SAD = (u32)&data;
    REG_DMA2DAD = (u32)&dma_dst_32;
    REG_DMA2CNT = DMA_ENABLE | DMA16 | 1;
    while (REG_DMA2CNT & DMA_ENABLE) ;

    // 32-bit DMA1 from IWRAM sets the DMA1 latch to 0xDEADC0DE
    data = 0xDEADC0DE;
    REG_DMA1SAD = (u32)&data;
    REG_DMA1DAD = (u32)&dma_dst_32;
    REG_DMA1CNT = DMA_ENABLE | DMA32 | 1;
    while ((REG_DMA1CNT) & DMA_ENABLE) ;

    // 16-bit DMA2 from 0x00000000 to dma_dst_1616[1]
    // During the read cycle the DMA2 latch is not updated because the source address is below 0x02000000.
    // During the write cycle the DMA2 latch MSW (0x1234) is written because the destination address isn't word-aligned.
    data = 0; // Reset data in case the old address is used in place of the invalid address
    REG_DMA2SAD = 0x00000000;
    REG_DMA2DAD = (u32)&dma_dst_1616[1];
    REG_DMA2CNT = DMA_ENABLE | DMA16 | 1;
    while (REG_DMA2CNT & DMA_ENABLE) ;

    test_expect_hex("DMA LATCH 2", 0x1234, dma_dst_1616[1]);
  }

  // Test that DMA from unused memory or read-only IO ports reads "regular" open bus.
  {
    // 32-bit DMA1 from 0x04000010 does update the DMA1 latch, but because 0x04000010 isn't a readable IO register,
    // this reads "regular" open bus (in this case the instruction prefetched right before the DMA activates).
    REG_DMA1SAD = 0x04000010;
    REG_DMA1DAD = (u32)&dma_dst_32;
    REG_DMA1CNT = DMA_ENABLE | DMA32 | 1;
    asm volatile(".hword 0x46C0; .hword 0x46C0; .hword 0x46C0; .hword 0x46C0;"); // NOP sled
    while (REG_DMA1CNT & DMA_ENABLE) ;

    test_expect_hex("BUS LATCH", 0x46C046C0, dma_dst_32);
  }

  test_print_metrics();

  while (1) {}
}


