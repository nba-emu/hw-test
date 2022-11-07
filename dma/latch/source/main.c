
#include <gba_console.h>
#include <gba_dma.h>
#include <stdio.h>

#include "test.h"

int main(void) {
  consoleDemoInit();

  u32 dest1 = 0;
  vu16 dest2[3] = {0xFFFF, 0xFFFF, 0xFFFF};

  // 1. DMA1 transfer 32-bit value 0xDEADC0DE (destination result discarded)
  // 2. DMA1 from DMA latch to dest2[0] (aligned)
  {
    u32 data = 0xDEADC0DE;

    REG_DMA1SAD = (u32)&data;
    REG_DMA1DAD = (u32)&dest1;
    REG_DMA1CNT = DMA_ENABLE | DMA32 | 1;

    while ((REG_DMA1CNT) & DMA_ENABLE) ;

    // In case the invalid address is not going to be latched.
    data = 0xAABBCCDD;

    REG_DMA1SAD = 0x00000000;
    REG_DMA1DAD = (u32)&dest2[0];
    REG_DMA1CNT = DMA_ENABLE | 1;

    while ((REG_DMA1CNT) & DMA_ENABLE) ;
  }

  // 1. DMA1 transfer 32-bit value 0xDEADC0DE (destination result discarded)
  // 2. DMA1 from DMA latch to dest2[1] (unaligned)
  {
    u32 data = 0xDEADC0DE;

    REG_DMA1SAD = (u32)&data;
    REG_DMA1DAD = (u32)&dest1;
    REG_DMA1CNT = DMA_ENABLE | DMA32 | 1;

    while ((REG_DMA1CNT) & DMA_ENABLE) ;

    // In case the invalid address is not going to be latched.
    data = 0xAABBCCDD;

    REG_DMA1SAD = 0x00000000;
    REG_DMA1DAD = (u32)&dest2[1];
    REG_DMA1CNT = DMA_ENABLE | 1;

    while ((REG_DMA1CNT) & DMA_ENABLE) ;
  }

  // 1. DMA1 transfer 32-bit value 0xDEADC0DE (destination result discarded)
  // 2. DMA2 from DMA latch to dest2[2] (aligned)
  {
    u32 data = 0xDEADC0DE;

    REG_DMA1SAD = (u32)&data;
    REG_DMA1DAD = (u32)&dest1;
    REG_DMA1CNT = DMA_ENABLE | DMA32 | 1;

    while ((REG_DMA1CNT) & DMA_ENABLE) ;

    // In case the invalid address is not going to be latched.
    data = 0xAABBCCDD;

    // Note: DMA2 latch technically is uninitialized, but the result already shows
    // that the latched is not shared anyways.
    REG_DMA2SAD = 0x00000000;
    REG_DMA2DAD = (u32)&dest2[2];
    REG_DMA2CNT = DMA_ENABLE | 1;

    while (REG_DMA2CNT & DMA_ENABLE) ;
  }

  test_expect_hex("DMA1 DMA1 EVEN", 0xC0DE, dest2[0]);
  test_expect_hex("DMA1 DMA1  ODD", 0xDEAD, dest2[1]);
  test_expect_hex("DMA1 DMA2 EVEN", 0x0000, dest2[2]);
  test_print_metrics();

  while (1) {
  }
}


