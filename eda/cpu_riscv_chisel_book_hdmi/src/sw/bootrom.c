#include <stddef.h>
#include <stdint.h>

extern void __attribute__((naked)) __attribute__((section(".isr_vector")))
isr_vector(void) {
  asm volatile("j _start");
  asm volatile("j _start");
}

void __attribute__((noreturn)) main(void);

extern void __attribute__((naked)) _start(void) {
  asm volatile("la sp, stack_top");
  main();
}

static uint64_t write_gpio_csr(uint32_t value) {
  asm volatile("csrw 0x7c0, %0" ::"r"(value));
}

static volatile uint32_t* const REG_GPIO_OUT = (volatile uint32_t*)0xA0000000;
static volatile uint32_t* const REG_VRAM =
    (volatile uint32_t*)0xB0000000;  // VRAMの先頭アドレス
static volatile uint32_t* const REG_VIDEO_CONTROLLER =
    (volatile uint32_t*)0xB0020000L;  // ビデオ・コントローラのレジスタ
// array (uint32_t[16])
static volatile uint32_t* const REG_COLOR_PALETTE =
    (volatile uint32_t*)0xB0020040L;  // ビデオ・コントローラのレジスタ
// 画面の幅
#define VIDEO_WIDTH (1280 / 16)
// 画面の高さ
#define VIDEO_HEIGHT (720 / 16)

void* __attribute__((inline)) memset(void* dest, int ch, size_t count) {
  uint8_t* dest_ = (uint8_t*)dest;
  while (count--) {
    *(dest_++) = ch;
  }
  return dest;
}

static void copy_from_vram(uint8_t* buffer, const uint32_t* vram, uint32_t xs,
                           uint32_t ys, uint32_t xe, uint32_t ye) {
  for (uint32_t y = ys; y < ye; y++) {
    const uint32_t* p = vram + y * VIDEO_WIDTH + xs;
    for (uint32_t x = xs; x < xe; x++) {
      *(buffer++) = *(p++);
    }
  }
}
static void copy_to_vram(uint32_t* vram, const uint8_t* buffer, uint32_t xs,
                         uint32_t ys, uint32_t xe, uint32_t ye) {
  for (uint32_t y = ys; y < ye; y++) {
    uint32_t* p = vram + y * VIDEO_WIDTH + xs;
    for (uint32_t x = xs; x < xe; x++) {
      *(p++) = *(buffer++);
    }
  }
}
static void fill_vram(uint32_t* vram, uint8_t value, uint32_t xs, uint32_t ys,
                      uint32_t xe, uint32_t ye) {
  for (uint32_t y = ys; y < ye; y++) {
    uint32_t* p = vram + y * VIDEO_WIDTH + xs;
    for (uint32_t x = xs; x < xe; x++) {
      *(p++) = value;
    }
  }
}

void __attribute__((noreturn)) main(void) {
  // 箱を描く矩形範囲の元の画像を保存しておくバッファ
  uint32_t led_out = 1;

  REG_COLOR_PALETTE[0] = 0x282c34;
  REG_COLOR_PALETTE[1] = 0xe06c75;
  REG_COLOR_PALETTE[2] = 0x98c379;
  REG_COLOR_PALETTE[3] = 0xe5c07b;
  REG_COLOR_PALETTE[4] = 0x61afef;
  REG_COLOR_PALETTE[5] = 0xc678dd;
  REG_COLOR_PALETTE[6] = 0x56b6c2;
  REG_COLOR_PALETTE[7] = 0xabb2bf;

  volatile uint32_t* vram_p = REG_VRAM;
  uint32_t width, color;
  for (uint32_t y = 0; y < VIDEO_HEIGHT; y++) {
    width = 0;
    color = 0;
    for (uint32_t x = 0; x < VIDEO_WIDTH; x++) {
      *(vram_p++) = color;
      ++width;
      if (width == VIDEO_WIDTH / 8) {
        width = 0;
        ++color;
      }
    }
  }

  uint32_t timer = 0;
  while (1) {
    // 垂直同期周波数が60[Hz]になっているので、
    // VSYNC待ちにより、ループは1/60[s]ごとに動作する
    while (*REG_VIDEO_CONTROLLER & 4)
      ;  // VSYNC待ち (VSYNCが0になるのを待つ)
    timer++;

    if (timer == 7200) {
      timer = 0;
      *REG_GPIO_OUT = led_out;
      write_gpio_csr(led_out);
      led_out = (led_out << 1) | ((led_out >> 5) & 1);

      REG_COLOR_PALETTE[8] = REG_COLOR_PALETTE[0];
      REG_COLOR_PALETTE[0] = REG_COLOR_PALETTE[1];
      REG_COLOR_PALETTE[1] = REG_COLOR_PALETTE[2];
      REG_COLOR_PALETTE[2] = REG_COLOR_PALETTE[3];
      REG_COLOR_PALETTE[3] = REG_COLOR_PALETTE[4];
      REG_COLOR_PALETTE[4] = REG_COLOR_PALETTE[5];
      REG_COLOR_PALETTE[5] = REG_COLOR_PALETTE[6];
      REG_COLOR_PALETTE[6] = REG_COLOR_PALETTE[7];
      REG_COLOR_PALETTE[7] = REG_COLOR_PALETTE[8];
    }
  }
}