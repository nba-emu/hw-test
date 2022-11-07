
#include <gba_base.h>

#include "emit.h"

#define WAIT_CYCLES 1024

static IWRAM_DATA u32 code_test[2048];
static IWRAM_DATA u32 code_wait[WAIT_CYCLES + 1];

static void emit_wait() {
  u32* code = &code_wait[0];

  for (int i = 0; i < WAIT_CYCLES; i++) {
    *code++ = 0xE320F000; // NOP
  }

  *code++ = 0xE12FFF1E; // BX LR
}

void emit_init() {
  emit_wait();
}

emit_fn emit_get_test(int delay, u32 address, u16* result) {
  u32* code = &code_test[0];

  // wait for 'delay' cycles before reading the address
  for (int i = 0; i < delay; i++) {
    *code++ = 0xE320F000; // NOP
  }

  // read from address
  *code++ = 0xE59F201C; // LDR R2, [PC, #28] (= address)
  *code++ = 0xE1D220B0; // LDRH R2, [R2]

  // write the result
  *code++ = 0xE59F1018; // LDR R1, [PC, #24] (= result)
  *code++ = 0xE1C120B0; // STRH R2, [R1]

  // set IE=0 (dumb way to signal the IRQ handler has completed)
  *code++ = 0xE3A00301; // MOV R0, #0x04000000
  *code++ = 0xE2800C02; // ADD R0, #0x200 (= 0x04000200)
  *code++ = 0xE3A01000; // MOV R1, #0
  *code++ = 0xE5801000; // STR R1, [R0]

  *code++ = 0xE12FFF1E; // BX LR

  *code++ = address;
  *code++ = (u32)result;

  return (emit_fn)&code_test[0];
}

emit_fn emit_get_wait() {
  return (emit_fn)&code_wait[0];
}