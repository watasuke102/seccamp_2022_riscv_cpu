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

static uint32_t uart_cplt(void) {
  uint32_t x;
  asm volatile("csrr %0, 0x7c2" : "=r"(x));
  return x;
}

static uint64_t uart_start(uint32_t value) {
  asm volatile("csrw 0x7c1, %0" ::"r"(value));
}

static volatile uint32_t* const REG_GPIO_OUT = (volatile uint32_t*)0xA0000000;

void __attribute__((noreturn)) main(void) {
  uint32_t led_out = 1;
  char* str = "Hello, RISC-V\r\n";
  int i = 0;

  uart_start('x');
  while (1) {
    if (uart_cplt()) {
      uart_start(str[i]);
      ++i;
      if (i > 14) {
        i = 0;
      }
    }

    // for (volatile uint32_t delay = 0; delay < 10000; delay++)
    //   ;
  }
}
