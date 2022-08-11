#include <stdbool.h>
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
static void delay_us(uint32_t duration_us) {
  uint64_t cycles = duration_us * 2;
  for (volatile int i = 0; i < cycles; ++i)
    ;
}

// LCD
static volatile uint32_t* const REG_LCD = (volatile uint32_t*)0xA0000020;
static void lcd_set_db(uint8_t db) {
  *REG_LCD = (*REG_LCD & 0xf0) | (db & 0x0f);
}
static void lcd_set_e(uint8_t e) {
  *REG_LCD = (*REG_LCD & ~0x10) | ((e != 0) << 4);
}
static void lcd_set_rs(uint8_t rs) {
  *REG_LCD = (*REG_LCD & ~0x20) | ((rs != 0) << 5);
}
static void lcd_set_rw(uint8_t rw) {
  *REG_LCD = (*REG_LCD & ~0x40) | ((rw != 0) << 6);
}

static void lcd_write_half(uint8_t data) {
  lcd_set_e(0);
  lcd_set_db(data);
  lcd_set_rw(0);
  delay_us(25);
  lcd_set_e(1);
  delay_us(3);
  lcd_set_e(0);
  delay_us(2);
}

static void lcd_write(bool is_data, uint8_t data) {
  lcd_set_rs(is_data);
  lcd_write_half(data >> 4);
  lcd_write_half(data & 0x0f);
}

static void lcd_init() {
  lcd_set_rs(0);
  delay_us(500000);
  lcd_write_half(0x03);
  delay_us(41000);
  lcd_write_half(0x03);
  delay_us(100);
  lcd_write_half(0x03);
  lcd_write_half(0x02);  // 4bit mode

  lcd_write(false, 0x28);  // 2 rows, 8 lines
  delay_us(37);
  lcd_write(false, 0x01);  // clear
  delay_us(20000);
  lcd_write(false, 0x06);  // entry mode set
  delay_us(37);
  lcd_write(false, 0x0f);  // show char / under line cursor / block cursor
  delay_us(37);
}

void __attribute__((noreturn)) main(void) {
  uint32_t led_out = 1;
  lcd_init();
  lcd_write(true, 'X');
  // lcd_write(false, 0x01);  // clear
  while (1) {
    *REG_GPIO_OUT = led_out;
    write_gpio_csr(led_out);
    led_out = (led_out << 1) | ((led_out >> 5) & 1);
    delay_us(500000);
  }
}
