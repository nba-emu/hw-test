#include <gba_console.h>
#include <gba_input.h>
#include <gba_video.h>
#include <stdio.h>

#include "ui.h"

static void set_palette(int row, int column, int palette) {
  u16* map_entry = (u16*)0x06002000 + row * 32 + column;

  *map_entry = (*map_entry & 0x0FFF) | (palette << 12);
}

void ui_init() {
  consoleDemoInit();

  //set black background color
  BG_COLORS[0] = RGB8(0, 0, 0);
}

void ui_clear() {
  printf("\e[1;1H\e[2J");
}

int ui_show_menu(UIMenuOption const* options, size_t length, bool may_return) {
  size_t option = 0;
  bool should_redraw = true;

  const size_t max_items = 20;

  size_t start = 0;

  while (true) {
    if (should_redraw) {
      ui_clear();

      for (size_t i = 0; i < max_items; i++) {
        size_t j = start + i;

        if(j == length) break;

        if (j == option) {
          printf("> %s", options[j].name);
        } else {
          printf("  %s", options[j].name);
        }

        if (i != max_items - 1) {
          printf("\n");
        }
      }

      should_redraw = false;
    }

    scanKeys();

    u16 keys_up = keysUp();

    if (keys_up & KEY_UP) {
      should_redraw = true;
      if (option == 0) {
        option = length - 1;

        if (length > max_items) {
          start = length - max_items;
        }
      } else {
        if (option == start) {
          start--;
        }

        option--;
      }
    }

    if (keys_up & KEY_DOWN) {
      should_redraw = true;
      option++;

      if (option == length) {
        option = 0;
        start = 0;
      } else if (option == (start + max_items)) {
        start++;
      }
    }

    if (keys_up & KEY_A) {
      void (*callback)() = options[option].callback;

      if (callback) {
        callback();
      }

      return option;
    }

    if ((keys_up & KEY_B) && may_return) {
      return -1;
    }
  }
}

void ui_view_bitmap_cmp(u8* bitmap_a, u8* bitmap_b, int length) {
  const int MAX_VISIBLE_ROWS = 18;

  const int PALETTE_FAIL = 13;
  const int PALETTE_PASS = 14;

  int scroll = 0;
  bool should_redraw = true;

  int rows = length / 16;

  if ((rows % 16) != 0) {
    rows++;
  }

  if (bitmap_b) {
    BG_COLORS[PALETTE_FAIL * 16 + 1] = RGB8(255, 0, 0);
    BG_COLORS[PALETTE_PASS * 16 + 1] = RGB8(0, 255, 0);
  }

  while (true) {
    if (should_redraw) {
      int index = scroll * 16;
      int visible_rows = rows - scroll;

      if (visible_rows > MAX_VISIBLE_ROWS) {
        visible_rows = MAX_VISIBLE_ROWS;
      }

      ui_clear();

      for (int row = 0; row < visible_rows; row++) {
        printf("%04d: ", index);

        for (int column = 0; column < 16; column++) {
          if (index >= length) {
            break;
          }

          int bit_a = bitmap_a[index] & 1;

          putchar(bit_a ? '1' : '0');
          
          if (bitmap_b) {
            int bit_b = bitmap_b[index] & 1;
            
            set_palette(
              row,
              column + 6,
              bit_a == bit_b ? PALETTE_PASS : PALETTE_FAIL
            );
          }

          index++;
        }

        if (index >= length) {
          break;
        }

        putchar('\n');
      }

      should_redraw = false;
    }

    scanKeys();

    u16 keys_up = keysUp();
    u16 keys_held = keysHeld();

    if (keys_up & KEY_UP) {
      should_redraw = true;

      if (keys_held & KEY_A) {
        scroll -= MAX_VISIBLE_ROWS;
      } else {
        scroll--;
      }

      if (scroll < 0) scroll = 0;
    }

    if (keys_up & KEY_DOWN) {
      should_redraw = true;

      if (keys_held & KEY_A) {
        scroll += MAX_VISIBLE_ROWS;
      } else {
        scroll++;
      }

      if (scroll >= rows) scroll = rows - 1;
    }

    if (keys_up & KEY_B) {
      return;
    }

    // TODO: V-sync
  }
}

void ui_view_bitmap(u8* bitmap, int length) {
  ui_view_bitmap_cmp(bitmap, NULL, length);
}
