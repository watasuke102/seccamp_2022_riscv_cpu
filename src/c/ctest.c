int main(void) {
  char str[] = "Hello, RISC-V\r\n";
  int i = 0;
  while (1) {
    ++i;
    if (i > sizeof(str)) {
      i = 0;
    }
    asm volatile("csrw 0x7c1, %0" ::"r"(str[i]));
    for (int i = 0; i < 8; ++i)
      ;
  }
  return 0;
}