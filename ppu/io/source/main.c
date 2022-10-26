#include <gba_base.h>
#include <gba_console.h>
#include <gba_interrupt.h>

#include "dispcnt.h"

#include "ui.h"

IWRAM_CODE int main(void) {
  consoleDemoInit();
  irqInit();

  while(true) {
    UIMenuOption options[] = {
      { "DISPCNT", &test_dispcnt }
    };

    ui_show_menu(options, sizeof(options) / sizeof(UIMenuOption), false);
  }
}
