#include <gba_base.h>
#include <gba_console.h>
#include <gba_dma.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <gba_video.h>
#include <stdio.h> 

#include "test.h"

IWRAM_CODE u32 crc32(vu8 const* data, int length) {
  u32 crc32 = 0xFFFFFFFF;

  while (length-- != 0) {
    u8 byte = *data++;

    for (int i = 0; i < 8; i++) {
      if ((crc32 ^ byte) & 1) {
        crc32 = (crc32 >> 1) ^ 0xEDB88320;
      }
      else {
        crc32 >>= 1;
      }
      byte >>= 1;
    }
  }

  return crc32;
}

IWRAM_CODE int main(void) {
  // initialize VRAM
  for (u32 address = 0x06000000; address < 0x06018000; address += 4) {
    *(vu32*)address = 0xFFFFFFFF;
  }

  u32 m0_10000h;
  u32 m0_14000h;
  u32 m0_18000h;
  u32 m0_1C000h;
  u32 m0_crc32;

  REG_DISPCNT = MODE_0 | BG2_ENABLE;
  *(vu32*)0x06018000 = 0xABAD1DEA;
  *(vu32*)0x0601C000 = 0x12345678;
  m0_10000h = *(vu32*)0x06010000;
  m0_14000h = *(vu32*)0x06014000;
  m0_18000h = *(vu32*)0x06018000;
  m0_1C000h = *(vu32*)0x0601C000;
  m0_crc32 = crc32((vu8 const*)0x06000000, 0x20000);

  u32 m3_10000h;
  u32 m3_14000h;
  u32 m3_18000h;
  u32 m3_1C000h;
  u32 m3_crc32;

  REG_DISPCNT = MODE_3 | BG2_ENABLE;
  *(vu32*)0x06018000 = 0xDEADBEEF;
  *(vu32*)0x0601C000 = 0x87654321;
  m3_10000h = *(vu32*)0x06010000;
  m3_14000h = *(vu32*)0x06014000;
  m3_18000h = *(vu32*)0x06018000;
  m3_1C000h = *(vu32*)0x0601C000;
  m3_crc32 = crc32((vu8 const*)0x06000000, 0x20000);

  consoleDemoInit();

  test_expect_hex("M0 10000h", 0xABAD1DEA, m0_10000h);
  test_expect_hex("M0 14000h", 0x12345678, m0_14000h);
  test_expect_hex("M0 18000h", 0xABAD1DEA, m0_18000h);
  test_expect_hex("M0 1C000h", 0x12345678, m0_1C000h);
  test_expect_hex("M0 CRC32", 0xD2E083BA, m0_crc32);

  test_expect_hex("M3 10000h", 0xABAD1DEA, m3_10000h);
  test_expect_hex("M3 14000h", 0x87654321, m3_14000h);
  test_expect_hex("M3 18000h", 0x00000000, m3_18000h);
  test_expect_hex("M3 1C000h", 0x87654321, m3_1C000h);
  test_expect_hex("M3 CRC32", 0x5B156CE9, m3_crc32);

  test_print_metrics();

  while (1) {
  }
}
