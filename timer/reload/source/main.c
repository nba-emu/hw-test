#include <gba_base.h>
#include <gba_console.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <gba_video.h>
#include <stdio.h> 

#include "../../../common.c"

IWRAM_CODE __attribute__((naked)) u16 test_reload(u16 reload) {
  asm(
    "ldr r1, =#0x04000100\n"

    // reset TM0
    "mov r3, #0\n"
    "str r3, [r1]\n"

    // start TM0 with given reload
    "mov r3, #0x00800000\n"
    "orr r3, r0\n"
    "str r3, [r1]\n"

    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"

    // read result and return
    "ldrh r0, [r1]\n"
    "bx lr\n"
  );
}

IWRAM_CODE __attribute__((naked)) u16 test_reload_overwrite_16(u16 reload) {
  asm(
    "ldr r1, =#0x04000100\n"
    "ldr r2, =#0xDEAD\n"

    // reset TM0
    "mov r3, #0\n"
    "str r3, [r1]\n"

    // start TM0 with given reload
    "mov r3, #0x00800000\n"
    "orr r3, r0\n"
    "str r3, [r1]\n"

    // overwrite RELOAD, then wait delay until overflow.
    "strh r2, [r1]\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"

    // read result and return
    "ldrh r0, [r1]\n"
    "bx lr\n"
  );
}

IWRAM_CODE __attribute__((naked)) u16 test_reload_overwrite_32(u16 reload) {
  asm(
    "ldr r1, =#0x04000100\n"
    "ldr r2, =#0x0080DEAD\n"

    // reset TM0
    "mov r3, #0\n"
    "str r3, [r1]\n"

    // start TM0 with given reload
    "mov r3, #0x00800000\n"
    "orr r3, r0\n"
    "str r3, [r1]\n"

    // overwrite RELOAD, then wait delay until overflow.
    "str r2, [r1]\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"

    // read result and return
    "ldrh r0, [r1]\n"
    "bx lr\n"
  );
}

IWRAM_CODE __attribute__((naked)) u16 test_reload_overwrite_32_late(u16 reload) {
  asm(
    "ldr r1, =#0x04000100\n"
    "ldr r2, =#0x0080DEAD\n"

    // reset TM0
    "mov r3, #0\n"
    "str r3, [r1]\n"

    // start TM0 with given reload
    "mov r3, #0x00800000\n"
    "orr r3, r0\n"
    "str r3, [r1]\n"

    // delay, then overwrite RELOAD in the last cycle
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "str r2, [r1]\n"

    // read result and return
    "ldrh r0, [r1]\n"
    "bx lr\n"
  );
}

IWRAM_CODE int main(void) {
  consoleDemoInit();

  expect_hex("FFF8 -- -", 0xFFF9, test_reload(0xFFF8));
  expect_hex("FFF8 16 0", 0xDEAE, test_reload_overwrite_16(0xFFF8));
  expect_hex("FFF8 32 0", 0xDEAE, test_reload_overwrite_32(0xFFF8));
  expect_hex("FFF8 32 7", 0xFFF9, test_reload_overwrite_32_late(0xFFF8));
  expect_hex("FFFF 16 0", 0xDEB4, test_reload_overwrite_16(0xFFFF));
  expect_hex("FFFF 32 0", 0xDEB4, test_reload_overwrite_32(0xFFFF));
  
  print_metrics();

  while (1) {
  }
}
