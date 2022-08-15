#include <gba_base.h>
#include <gba_console.h>
#include <gba_dma.h>
#include <gba_interrupt.h>
#include <gba_timers.h>
#include <gba_video.h>
#include <stdio.h>

static IWRAM_DATA u32 code_buffer[4096];

void test_mode3_stalls(int line) {
  int vcount_a;
  int vcount_b;

  if (line == 0) {
    vcount_a = 226;
    vcount_b = 227;
  } else if (line == 1) {
    vcount_a = 227;
    vcount_b = 0;
  } else {
    vcount_a = line - 2;
    vcount_b = line - 1;
  }

  REG_DISPCNT = MODE_3 | BG2_ENABLE;

  REG_IE = 0;
  irqSet(IRQ_HBLANK, (void (*)())code_buffer);

  u16 results[1232];

  for (int cycle = 0; cycle < 1232; cycle++) {
    REG_DISPSTAT &= ~LCDC_HBL;
    REG_IME = 0;
    REG_IE = IRQ_HBLANK;
    REG_IF = 0xFFFF;

    u32* code = code_buffer;

    // IWWRAM IRQ handler codegen
    {
      // start TM0 timer:
      *code++ = 0xE3A00301; // MOV R0, #0x04000000
      *code++ = 0xE2800C01; // ADD R0, #0x100 (= 0x04000100)
      *code++ = 0xE3A01080; // MOV R1, #0x80
      *code++ = 0xE1C010B2; // STRH R1, [R0, #2]

      // wait for 'cycle' cycles before writing to VRAM
      for (int i = 0; i < cycle; i++) {
        *code++ = 0xE320F000; // NOP
      }

      // write to background VRAM:
      *code++ = 0xE3A02406; // MOV R2, #0x06000000
      *code++ = 0xE1C220B0; // STRH R2, [R2]

      // stop TM0 timer:
      *code++ = 0xE3A01000; // MOV R1, #0
      *code++ = 0xE1C010B2; // STRH R1, [R0, #2]

      // set IE=0 (dumb way to signal the IRQ handler has completed)
      *code++ = 0xE2800C01; // ADD R0, #0x100 (= 0x04000200)
      *code++ = 0xE5801000; // STR R1, [R0]

      *code++ = 0xE12FFF1E; // BX LR
    }

    void (*wait)() = code;

    // IWRAM wait function
    // this function is called to make sure, that the IRQ can be handled
    // on the very cycle that it is signalled to the CPU:
    {
      for (int i = 0; i < 1024; i++) {
        *code++ = 0xE320F000; // NOP
      }

      *code++ = 0xE12FFF1E; // BX LR
    }

    REG_TM0CNT = 0;

    while (REG_VCOUNT != vcount_a);
    while (REG_VCOUNT != vcount_b);
  
    REG_DISPSTAT |= LCDC_HBL;
    REG_IME = 1;
    wait(); // let the IRQ happen
    while (REG_IE != 0) {} // this might be useless at this point

    results[cycle] = REG_TM0CNT_L;
  }

  int a = 0;
  int b = 0;
  int c = 0;

  u8* sram = (u8*)0x0E000000;

  int calibration = (int)results[0];

  for (int cycle = 1; cycle < 1232; cycle++) {
    int stalls = results[cycle] - calibration - cycle;

    if (stalls == 0) {
      a++;
    } else if (stalls == 1) {
      b++;
    } else {
      c++;
    }

    sram[cycle] = (u8)stalls;
  }

  consoleDemoInit();
  printf("%d %d %d\n", a, b, c);
  puts("done nya? :3");
}

int main(void) {
  irqInit();

  test_mode3_stalls(0);

  while (true) {
  }
}
