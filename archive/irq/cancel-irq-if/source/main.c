#include <gba_base.h>
#include <gba_console.h>
#include <gba_timers.h>
#include <gba_interrupt.h>
#include <stdio.h>

static volatile bool irq_handled;

IWRAM_CODE void irq_handler() {
  REG_TM0CNT = 0;
  irq_handled = true;
}

IWRAM_CODE __attribute__((noinline)) void __test_cancel_if(int nops) {
  asm(
    "push {r4}\n"

    // convert number of nops to run to number of nops to skip
    "rsb r0, r0, #8\n"

    // prepare for clearing IF
    "ldr r3, =#0xFFFF\n"
    "ldr r4, =#0x04000202\n"

    // start TM0 with IRQ enabled
    "ldr r1, =#0x04000102\n"
    "mov r2, #0xC0\n"
    "strh r2, [r1]\n"

    // jump into the NOP sled
    "add pc, r0, lsl #2\n"
    "nop\n"
    "nop\n"

    // NOP sled
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"

    // clear IF
    "strh r3, [r4]\n"

    // make sure the IRQ is taken as soon as possible
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"

    "pop {r4}\n"
    "bx lr\n"
    ".pool"
  );
}

IWRAM_CODE __attribute__((noinline)) void test_cancel_if() {
  static const bool TEST_RESULTS[8] = {
    true,
    true,
    true,
    true,
    true,
    true,
    true,
    true
  };

  for (int i = 0; i < 8; i++) {
    // reset TM0 with reload=0xFFF8:
    REG_TM0CNT = 0xFFFC;

    // acknowledge all IRQs and enable TM0 IRQ:
    REG_IF = 0xFFFF;
    REG_IE = IRQ_TIMER0;
    REG_IME = 1;

    // reset IRQ handled flag
    irq_handled = false;

    __test_cancel_if(i);

    // wait for the IRQ with a timeout
    for (int j = 0; j < 1024; j++) {
      if (irq_handled) break;
    }

    // print test result:
    printf("#%d NOPs ", i);
    printf(irq_handled ? "IRQ " : "--- ");
    puts(irq_handled == TEST_RESULTS[i] ? "PASS" : "FAIL");
  }
}

IWRAM_CODE int main(void) {
  consoleDemoInit();

  irqInit();
  irqSet(IRQ_TIMER0, irq_handler);
  REG_IE = 0;
  REG_IF = 0xFFFF;

  test_cancel_if();

  puts("\nIE");

  while (1) {
  }
}
