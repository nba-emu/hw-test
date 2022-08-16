
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

emit_fn emit_get_test(int delay, u32 address) {
  u32* code = &code_test[0];

  // start TM0 timer:
  *code++ = 0xE3A00301; // MOV R0, #0x04000000
  *code++ = 0xE2800C01; // ADD R0, #0x100 (= 0x04000100)
  *code++ = 0xE3A01080; // MOV R1, #0x80
  *code++ = 0xE1C010B2; // STRH R1, [R0, #2]

  // wait for 'delay' cycles before writing to VRAM
  for (int i = 0; i < delay; i++) {
    *code++ = 0xE320F000; // NOP
  }

  // read from VRAM/PRAM/OAM:
  //*code++ = 0xE3A02406; // MOV R2, #0x06000000
  //*code++ = 0xE1C220B0; // STRH R2, [R2]
  *code++ = 0xE59F2014; // LDR R2, [PC, #20] (= address)
  *code++ = 0xE1D220B0; // LDRH R2, [R2]

  // stop TM0 timer:
  *code++ = 0xE3A01000; // MOV R1, #0
  *code++ = 0xE1C010B2; // STRH R1, [R0, #2]

  // set IE=0 (dumb way to signal the IRQ handler has completed)
  *code++ = 0xE2800C01; // ADD R0, #0x100 (= 0x04000200)
  *code++ = 0xE5801000; // STR R1, [R0]

  *code++ = 0xE12FFF1E; // BX LR

  *code++ = address;

  return (emit_fn)&code_test[0];
}

emit_fn emit_get_wait() {
  return (emit_fn)&code_wait[0];
}