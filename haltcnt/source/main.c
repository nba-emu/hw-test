#include <gba_base.h>
#include <gba_console.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <gba_video.h>
#include <stdio.h> 

#include "../../common.c"

#define REG_POSTFLG *((vu8*)0x04000300)
#define REG_HALTCNT *((vu8*)0x04000301)

IWRAM_CODE void cpu_irq_disable() {
  asm(
    "mrs r0, cpsr\n"
    "orr r0, #0x80\n"
    "msr cpsr_c, r0\n"
  );
}

IWRAM_CODE void test_haltcnt_direct() {
  // synchronize to scanline start
  while (REG_VCOUNT != 0) ;
  while (REG_VCOUNT != 1) ;

  // acknowledge all pending IRQs
  REG_IF = 0xFFFF;

  // start timer
  REG_TM0CNT_H = 0;
  REG_TM0CNT_L = 0;
  REG_TM0CNT_H = TIMER_START;

  // wait until H-blank IRQ
  REG_HALTCNT = 0;

  u16 result = REG_TM0CNT_L;

  expect_range("HALTCNT DIRECT", 0, 16, result);
}

IWRAM_CODE void test_haltcnt_cpuset() {
  // synchronize to scanline start
  while (REG_VCOUNT != 0) ;
  while (REG_VCOUNT != 1) ;

  // acknowledge all pending IRQs
  REG_IF = 0xFFFF;

  // start timer
  REG_TM0CNT_H = 0;
  REG_TM0CNT_L = 0;
  REG_TM0CNT_H = TIMER_START;

  // wait until H-blank IRQ
  // HALTCNT is in the upper byte of POSTFLG
  u16 tmp = 0;
  CpuSet(&tmp, (void*)&REG_POSTFLG, 1);

  u16 result = REG_TM0CNT_L;

  expect_range("HALTCNT CPUSET", 900, 1100, result);

  // make sure that the CpuSet doesn't unset POSTFLAG.
  expect("POSTFLG", 1, REG_POSTFLG);
}

__attribute__((optimize("O0"))) IWRAM_CODE void test_haltcnt_timing_iwram() {
  u16 tmp;

  REG_IME = 0;
  REG_IE = IRQ_TIMER0;
  REG_IF = 0xFFFF;
  
  // reset timer and wait until it asserted an IRQ
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START | TIMER_IRQ;
  
  while ((REG_IF & IRQ_TIMER0) == 0);

  // reset timer again (avoid that overflow messes with the timing or anything)
  // then enter HALT mode with the already-asserted IRQ
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  IWRAM_CpuSet(&tmp, (void*)&REG_POSTFLG, 1);
  
  u16 timer = REG_TM0CNT_L;
  expect("HALTCNT TIME IWRAM", 125, timer);
}

__attribute__((optimize("O0"))) void test_haltcnt_timing_rom() {
  u16 tmp;

  REG_IME = 0;
  REG_IE = IRQ_TIMER0;
  REG_IF = 0xFFFF;

  // reset timer and wait until it asserted an IRQ
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START | TIMER_IRQ;

  while ((REG_IF & IRQ_TIMER0) == 0);

  // reset timer again (avoid that overflow messes with the timing or anything)
  // then enter HALT mode with the already-asserted IRQ
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;
  CpuSet(&tmp, (void*)&REG_POSTFLG, 1);

  u16 timer = REG_TM0CNT_L;
  expect("HALTCNT TIME ROM", 249, timer);
}

IWRAM_CODE int main(void) {
  consoleDemoInit();

  // enable H-blank IRQ and disable all other IRQs
  cpu_irq_disable();  // disable actual CPU IRQs as well
  REG_IME = 0;
  REG_IE = IRQ_HBLANK;
  REG_DISPSTAT = LCDC_HBL;

  test_haltcnt_direct();
  test_haltcnt_cpuset();
  test_haltcnt_timing_iwram();
  test_haltcnt_timing_rom();
  print_metrics();

  while (1) {
  }
}
