#include <gba_base.h>
#include <gba_console.h>
#include <gba_timers.h>
#include <gba_input.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_video.h>
#include <stdio.h>

#include "test.h"

static volatile bool irq_handled;
static u16 timer0_value = 0;

IWRAM_CODE void irq_handler() {
  if (!irq_handled) {
    timer0_value = REG_TM0CNT_L;
    irq_handled = true;
    REG_TM0CNT = 0;
  }
}

#define ALWAYS_INLINE inline __attribute__((always_inline))

static ALWAYS_INLINE void __test_irq_time(const char* test_name, u16 expected) {
  // disable IRQs to avoid one being handled before we want it to.
  REG_IME = 0;
  REG_IE = 0;
  REG_IF = 0xFFFF;

  irqSet(IRQ_TIMER0, irq_handler);
  REG_IE = IRQ_TIMER0;

  REG_TM0CNT = 0xFFFE;
  REG_TM0CNT_H = TIMER_START | TIMER_IRQ;

  // We use a NOP sled to be sure, that enabling the timer,
  // or raising the timer IRQ is not going to interfere with the timing 
  // to actually serve the IRQ.
  asm volatile(
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
  );

  // Reset the timer, which previously asserted the IRQ.
  // We will now use it to measure how long the CPU takes to serve that IRQ.
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;

  // reenable IRQs and wait for the IRQ to be handled.
  irq_handled = false;
  REG_IME = 1;
  while (!irq_handled) ;

  test_expect(test_name, expected, timer0_value);
}

static __attribute__((noinline)) IWRAM_CODE void test_irq_time_iwram() {
  __test_irq_time("IRQ TIME IWRAM", 92);
}

static __attribute__((noinline)) EWRAM_CODE void test_irq_time_ewram() {
  __test_irq_time("IRQ TIME EWRAM", 112);
}

static __attribute__((noinline)) void test_irq_time_rom() {
  __test_irq_time("IRQ TIME ROM", 120);
}

IWRAM_CODE int main(void) {
  consoleDemoInit();
  irqInit();
  
  test_irq_time_iwram();
  test_irq_time_ewram();
  test_irq_time_rom();
  
  test_print_metrics();

  while (1) {
  }
}
