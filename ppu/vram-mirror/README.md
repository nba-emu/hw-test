# vram-mirror

This test checks VRAM mirror (`0x06018000` - `0x0601FFFF`) read/write behaviour differences between text-based modes and bitmap modes.

In bitmap modes reads and writes to `0x06018000` - `0x0601BFFF` do not work (writes are discarded; reads may always return 0?).

It appears that VRAM accesses above `0x06000000` (text-based modes) or `0x06014000` (bitmap-based modes) are always passed through the OBJ engine,
which in bitmap-based modes cannot access the VRAM bank at `0x06010000` - `0x06013FFF` (mirrored at `0x06018000`), since it is allocated to the BG engine.

This test does **not** pass on a Nintendo 3DS in GBA mode (tested via open-agb-firm), but it passes on GBA and GBA SP.
