
#include <gba_console.h>
#include <stdio.h>

#include "../../../common.c"

int main(void) {
	consoleDemoInit();

	u32 dest1 = 0;
	vu16 dest2[3] = {0xFFFF, 0xFFFF, 0xFFFF};

	// 1. DMA1 transfer 32-bit value 0xDEADC0DE (destination result discarded)
	// 2. DMA1 from DMA latch to dest2[0] (aligned)
	{
		u32 data = 0xDEADC0DE;

		*(vu32*)0x040000BC = (u32)&data;
		*(vu32*)0x040000C0 = (u32)&dest1;
		*(vu32*)0x040000C4 = (1 << 31) | (1 << 26) | 1;

		while ((*(vu32*)0x040000C4) & (1 << 31)) ;

		// In case the invalid address is not going to be latched.
		data = 0xAABBCCDD;

		*(vu32*)0x040000BC = 0x00000000;
		*(vu32*)0x040000C0 = (u32)&dest2[0];
		*(vu32*)0x040000C4 = (1 << 31) | 1;

		while ((*(vu32*)0x040000C4) & (1 << 31)) ;
	}

	// 1. DMA1 transfer 32-bit value 0xDEADC0DE (destination result discarded)
	// 2. DMA1 from DMA latch to dest2[1] (unaligned)
	{
		u32 data = 0xDEADC0DE;

		*(vu32*)0x040000BC = (u32)&data;
		*(vu32*)0x040000C0 = (u32)&dest1;
		*(vu32*)0x040000C4 = (1 << 31) | (1 << 26) | 1;

		while ((*(vu32*)0x040000C4) & (1 << 31)) ;

		// In case the invalid address is not going to be latched.
		data = 0xAABBCCDD;

		*(vu32*)0x040000BC = 0x00000000;
		*(vu32*)0x040000C0 = (u32)&dest2[1];
		*(vu32*)0x040000C4 = (1 << 31) | 1;

		while ((*(vu32*)0x040000C4) & (1 << 31)) ;
	}

	// 1. DMA1 transfer 32-bit value 0xDEADC0DE (destination result discarded)
	// 2. DMA2 from DMA latch to dest2[2] (aligned)
	{
		u32 data = 0xDEADC0DE;

		*(vu32*)0x040000BC = (u32)&data;
		*(vu32*)0x040000C0 = (u32)&dest1;
		*(vu32*)0x040000C4 = (1 << 31) | (1 << 26) | 1;

		while ((*(vu32*)0x040000C4) & (1 << 31)) ;

		// In case the invalid address is not going to be latched.
		data = 0xAABBCCDD;

		// Note: DMA2 latch technically is uninitialized, but the result already shows
		// that the latched is not shared anyways.
		*(vu32*)0x040000C8 = 0x00000000;
		*(vu32*)0x040000CC = (u32)&dest2[2];
		*(vu32*)0x040000D0 = (1 << 31) | 1;

		while ((*(vu32*)0x040000D0) & (1 << 31)) ;
	}

	expect("DMA1 DMA1 EVEN", 0xC0DE, dest2[0]);
	expect("DMA1 DMA1  ODD", 0xDEAD, dest2[1]);
	expect("DMA1 DMA2 EVEN", 0x0000, dest2[2]);
	print_metrics();

	while (1) {
	}
}


