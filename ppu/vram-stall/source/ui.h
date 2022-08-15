
#include <gba_base.h>

#ifndef _UI_H_
#define _UI_H_

typedef struct {
  const char* name;
  void (*callback)();
} UIMenuOption;

int ui_show_menu(UIMenuOption const* options, size_t length, bool may_return);

void ui_view_bitmap(u8* bitmap, int length);
void ui_view_bitmap_cmp(u8* bitmap_a, u8* bitmap_b, int length);

#endif // _UI_H_
