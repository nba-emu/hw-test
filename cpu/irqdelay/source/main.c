#include <gba_base.h>
#include <gba_console.h>
#include <gba_timers.h>
#include <gba_input.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_video.h>
#include <stdio.h>

#include "../../../common.c"

#define REG_POSTFLG *((vu8*)0x04000300)

static volatile int counter = 0;
static u16 buffer[10];

IWRAM_CODE void timer_irq_handler() {
  u16 cycle_count = REG_TM0CNT_L;

  buffer[counter] = cycle_count;

  if (++counter == 10) {
    irqDisable(IRQ_TIMER0);
  }
}

IWRAM_CODE void test_timer_irq_halt() {
  REG_IME = 0;
  REG_DISPSTAT = 0;
  REG_IE = IRQ_TIMER0;
  REG_IF = 0xFFFF;

  REG_TM0CNT_H = 0;
  REG_TM0CNT_L = 0x8000;
  REG_TM0CNT_H = TIMER_START | TIMER_IRQ;

  // wait until timer #0 IRQ
  // HALTCNT is in the upper byte of POSTFLG
  u16 tmp = 0;
  CpuSet(&tmp, (void*)&REG_POSTFLG, 1);

  u16 timer_value = REG_TM0CNT_L;

  printf("%04x\n\n", timer_value);
}

IWRAM_CODE void test_timer_irq_cpu_irq() {
  REG_TM0CNT_H = 0;
  REG_IF = 0xFFFF;

  irqSet(IRQ_TIMER0, timer_irq_handler);
  irqEnable(IRQ_TIMER0);

  REG_TM0CNT_L = 0x8000;
  REG_TM0CNT_H = TIMER_START | TIMER_IRQ;

  while (counter != 10) ;

  for (int i = 0; i < 10; i++) {
    printf("%04x\n", buffer[i]);
  }

  puts("");
}

IWRAM_CODE void test_ppu_hblank_halt_sync_dispstat() {
  REG_IME = 0;
  REG_DISPSTAT = LCDC_HBL;
  REG_IE = IRQ_HBLANK;

  REG_TM0CNT_H = 0;
  REG_TM0CNT_L = 0;

  // wait for start of H-draw
  while (!(REG_DISPSTAT & LCDC_HBL_FLAG)) ;
  while (  REG_DISPSTAT & LCDC_HBL_FLAG ) ;

  REG_IF = 0xFFFF;
  REG_TM0CNT_H = TIMER_START;

  // wait until H-blank IRQ
  // HALTCNT is in the upper byte of POSTFLG
  u16 tmp = 0;
  CpuSet(&tmp, (void*)&REG_POSTFLG, 1);

  u16 timer_value = REG_TM0CNT_L;

  printf("%d\n\n", timer_value);
}

IWRAM_CODE void test_ppu_hblank_halt_sync_vcount() {
  REG_IME = 0;
  REG_DISPSTAT = LCDC_HBL;
  REG_IE = IRQ_HBLANK;

  REG_TM0CNT_H = 0;
  REG_TM0CNT_L = 0;

  // wait for start of H-draw
  while (REG_VCOUNT != 63) ;
  while (REG_VCOUNT != 64) ;

  REG_IF = 0xFFFF;
  REG_TM0CNT_H = TIMER_START;

  // wait until H-blank IRQ
  // HALTCNT is in the upper byte of POSTFLG
  u16 tmp = 0;
  CpuSet(&tmp, (void*)&REG_POSTFLG, 1);

  u16 timer_value = REG_TM0CNT_L;

  printf("%d\n\n", timer_value);
}

IWRAM_CODE int main(void) {
  consoleDemoInit();
  irqInit();

  test_timer_irq_halt();
  test_timer_irq_cpu_irq();

  test_ppu_hblank_halt_sync_dispstat();
  test_ppu_hblank_halt_sync_vcount();

  while (1) {
  }
}
