# vram-mirror

This test checks VRAM mirror (`0x06018000` - `0x0601FFFF`) read/write behaviour differences between text-based modes and bitmap modes.

In bitmap modes reads and writes to `0x06010000` - `0x06013FFF` do not work (writes are discarded; reads may always return 0?),
as that VRAM bank, which normally is allocated to the sprite renderer, is allocated to the background renderer.

This test does **not** pass on a Nintendo 3DS in GBA mode (tested via open-agb-firm), but it passes on GBA and GBA SP.