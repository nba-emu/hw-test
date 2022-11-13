#include <gba_base.h>
#include <gba_console.h>
#include <gba_interrupt.h>
#include <gba_input.h>
#include <gba_systemcalls.h>
#include <gba_video.h>
#include <gba_timers.h>
#include <gba_dma.h>
#include <stdio.h>

#include "emit.h"
#include "test.h"
#include "ui.h"

void press_b() {
  // while(REG_KEYINPUT & KEY_B) VBlankIntrWait();

  while(REG_KEYINPUT & KEY_B) ;
}

// @todo: make this reusable
static void sync(int line) {
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

  while (REG_VCOUNT != vcount_a);
  while (REG_VCOUNT != vcount_b);
}

static u16 __test_cycle(int line, u32 address, int cycle, emit_fn wait) {
  u16 result = 0xDEAD;

  // Make sure no IRQ is requested or handled,
  // while we prepare for a H-blank IRQ.
  REG_IME = 0;
  REG_DISPSTAT &= ~LCDC_HBL;

  // Register a H-blank IRQ handler with a delay of 'cycle' cycles.
  irqSet(IRQ_HBLANK, emit_get_test(cycle, address, &result));

  // Sync to the start of the scanline, before the one we're targeting.
  sync(line);

  // Acknowledge all pending IRQs and allow the next H-blank IRQ to happen.
  REG_IE = IRQ_HBLANK;
  REG_IF = 0xFFFF;
  REG_IME = 1;
  REG_DISPSTAT |= LCDC_HBL;

  /* Run a sequence of 1024 nops in IWRAM,
   * this will allow the IRQ to be taken immediately,
   * once the CPU registers it.
   */
  wait();

  // Wait for the IRQ handler to complete execution.
  while (REG_IE != 0) ;

  return result;
}

void test_dispstat() {
  const int lines[] = {0, 160, 227};

  // u16 results[3][1232];

  int hblank_0 = -1;
  int hblank_1 = -1;
  int vcount_match = -1;
  int vblank_0 = -1;
  int vblank_1 = -1;

  ui_clear();

  emit_fn wait = emit_get_wait();

  // match on VCOUNT==0
  REG_DISPSTAT = REG_DISPSTAT & 0xFF;

  for(int i = 0; i < sizeof(lines)/sizeof(int); i++) {
    int line = lines[i];

    u16 dispstat_old = 0;

    for(int cycle = 0; cycle < 1232; cycle++) {
      u16 dispstat_new = __test_cycle(line, 0x04000004, cycle, wait);

      if(cycle > 0) {
        // check h-blank and v-count match flags on line #0
        if(line == 0) {
          if((dispstat_old & LCDC_HBL_FLAG) && !(dispstat_new & LCDC_HBL_FLAG)) {
            hblank_0 = cycle;
          }

          if(!(dispstat_old & LCDC_HBL_FLAG) && (dispstat_new & LCDC_HBL_FLAG)) {
            hblank_1 = cycle;
          }

          if((dispstat_old & LCDC_VCNT_FLAG) != (dispstat_new & LCDC_VCNT_FLAG)) {
            vcount_match = cycle;
          }
        }

        // check v-blank=1 on line #160
        if(line == 160 && !(dispstat_old & LCDC_VBL_FLAG) && (dispstat_new & LCDC_VBL_FLAG)) {
          vblank_1 = cycle;
          break;
        }

        if(line == 227 && (dispstat_old & LCDC_VBL_FLAG) && !(dispstat_new & LCDC_VBL_FLAG)) {
          vblank_0 = cycle;
          break;
        }
      }

      dispstat_old = dispstat_new;
    }
  }

  test_reset();
  test_expect("HBLANK=0",  144, hblank_0);
  test_expect("HBLANK=1", 1151, hblank_1);
  test_expect("VMATCH=1",  145, vcount_match);
  test_expect("VBLANK=0",  144, vblank_0);
  test_expect("VBLANK=1",  144, vblank_1);
  test_print_metrics();

  press_b();
}

void test_vcount() {
  emit_fn wait = emit_get_wait();
  
  int vcount_inc = -1;

  ui_clear();

  for(int cycle = 0; cycle < 1232; cycle++) {
    const int line = 1;

    u16 vcount = __test_cycle(line, 0x04000006, cycle, wait);

    if(vcount == line) {
      vcount_inc = cycle;
      break;
    }
  }

  test_reset();
  test_expect("VCOUNT INC", 144, vcount_inc);
  test_print_metrics();

  press_b();
}

void test_if() {
  const int line = 160;

  emit_fn wait = emit_get_wait();

  u16 old_if = 0;
  int hblank = -1;
  int vblank = -1;
  int vmatch = -1;

  // enable h-blank, v-blank and v-match IRQs and match on VCOUNT==160
  REG_DISPSTAT = (REG_DISPSTAT & 0xFF) | (line << 8) | LCDC_HBL | LCDC_VBL | LCDC_VCNT;

  ui_clear();

  for(int cycle = 0; cycle < 1232; cycle++) {
    u16 new_if = __test_cycle(line, 0x04000202, cycle, wait);

    if(cycle > 0) {
      if(!(old_if & IRQ_HBLANK) && (new_if & IRQ_HBLANK)) {
        hblank = cycle;
      }

      if(!(old_if & IRQ_VBLANK) && (new_if & IRQ_VBLANK)) {
        vblank = cycle;
      }

      if(!(old_if & IRQ_VCOUNT) && (new_if & IRQ_VCOUNT)) {
        vmatch = cycle;
      }
    }

    old_if = new_if;
  }

  test_reset();
  test_expect("HBLANK", 1153, hblank);
  test_expect("VBLANK",  146, vblank);
  test_expect("VMATCH",  147, vmatch);
  test_print_metrics();

  press_b();
}

static volatile bool irq_done;

u16 hblank_dma_time;
u16 vblank_dma_time;
u16 video_dma_time;

static IWRAM_CODE __attribute__((optimize("O0"))) 
void __test_dma_hblank_irq_handler() {
  REG_IE = 0;
  REG_TM0CNT = 0;
  REG_TM0CNT_H = TIMER_START;

  // DMA0 reads TM0 at H-blank
  REG_DMA0SAD = (u32)&REG_TM0CNT;
  REG_DMA0DAD = (u32)&hblank_dma_time;
  REG_DMA0CNT = DMA_ENABLE | DMA16 | DMA_HBLANK | 1;

  // DMA1 reads TM0 at V-blank
  REG_DMA1SAD = (u32)&REG_TM0CNT;
  REG_DMA1DAD = (u32)&vblank_dma_time;
  REG_DMA1CNT = DMA_ENABLE | DMA16 | DMA_VBLANK | 1;

  // DMA3 reads TM0 at line #0 video transfer DMA
  REG_DMA3SAD = (u32)&REG_TM0CNT;
  REG_DMA3DAD = (u32)&video_dma_time;
  REG_DMA3CNT = DMA_ENABLE | DMA16 | DMA_SPECIAL | 1;

  while(REG_DMA0CNT & DMA_ENABLE) ;
  while(REG_DMA1CNT & DMA_ENABLE) ;
  while(REG_DMA3CNT & DMA_ENABLE) ;

  irq_done = true;
}

void test_dma() {
  // Make sure no IRQ is requested or handled,
  // while we prepare for a H-blank IRQ.
  REG_IME = 0;
  REG_DISPSTAT &= ~LCDC_HBL;

  hblank_dma_time = -1;
  vblank_dma_time = -1;
  video_dma_time  = -1;

  // Register our H-blank IRQ handler
  irqSet(IRQ_HBLANK, &__test_dma_hblank_irq_handler);

  // Sync to line #159
  sync(159);

  irq_done = false;

  // Acknowledge all pending IRQs and allow the next H-blank IRQ to happen.
  REG_IE = IRQ_HBLANK;
  REG_IF = 0xFFFF;
  REG_IME = 1;
  REG_DISPSTAT |= LCDC_HBL;

  /* Run a sequence of 1024 nops in IWRAM,
   * this will allow the IRQ to be taken immediately,
   * once the CPU registers it.
   */
  emit_get_wait()();

  while(!irq_done) ;

  ui_clear();
  test_reset();
  test_expect("HBLANK DMA", 1137, hblank_dma_time);
  test_expect("VBLANK DMA", 1362, vblank_dma_time);
  test_expect("VIDEO DMA", 22069, video_dma_time);

  press_b();
}

int main(void) {
  irqInit();

  ui_init();
  emit_init();

  while (true) {
    UIMenuOption options[] = {
      { "DISPSTAT", &test_dispstat },
      { "VCOUNT", &test_vcount },
      { "IRQ (IF)", &test_if },
      { "DMA", &test_dma }
    };

    ui_show_menu(options, sizeof(options) / sizeof(UIMenuOption), false);

    // VBlankIntrWait();
  }
}
