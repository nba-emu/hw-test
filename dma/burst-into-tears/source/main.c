
#include <gba_console.h>
#include <gba_dma.h>
#include <gba_timers.h>
#include <stdio.h>

u16 IWRAM_CODE run_dma(u32 src, u32 dst, u16 count, u32 flags) {
	REG_TM0CNT_H = 0;
	REG_TM0CNT_L = 0;
	REG_TM0CNT_H = TIMER_START;

	REG_DMA3SAD = src;
	REG_DMA3DAD = dst;
	REG_DMA3CNT = DMA_ENABLE | DMA16 | DMA_IMMEDIATE | count | flags;

	asm(
		"nop\n"
		"nop\n"
		"nop\n"
	);

	return REG_TM0CNT_L;
}

int IWRAM_CODE main(void) {
	const int time_expect = 67;

	consoleDemoInit();

	int time_real = run_dma(0x07FFFFFE, 0x08000000, 3, DMA_SRC_INC | DMA_DST_DEC);
	u16 a_real = *(u16*)0x07FFFFFE;
	u16 b_real = *(u16*)0x07FFFFFC;

	u16 a_expect = *(u16*)0x08000002;
	u16 b_expect = *(u16*)0x08000004;

	printf("1ST VALUE:\n\t");
	if (a_real != a_expect) {
		printf("FAIL %04X != %04X\n", a_real, a_expect);
	} else {
		printf("PASS %04X\n", a_real);
	}

	printf("\n2ND VALUE:\n\t");
	if (b_real != b_expect) {
		printf("FAIL %04X != %04X\n", b_real, b_expect);
	} else {
		printf("PASS %04X\n", b_real);
	}

	printf("\nDMA TIME:\n\t");
	if (time_real != time_expect) {
		printf("FAIL %d != %d\n", time_real, time_expect);
	} else {
		printf("PASS %d\n", time_real);
	}

	while (1) {
	}
}


