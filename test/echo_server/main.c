
static inline void outl(short port, int val) { __asm__ volatile("outl %0, %1" ::"a"(val), "dN"(port)); }

extern void record_timestamp(void);

char *append_string(char *s, const char *to_add) {
  for (int i = 0; to_add[i]; i++) {
    *(s++) = to_add[i];
  }
  return s;
}


__attribute__((noinline)) long hcall(int nr, unsigned long a, unsigned long b) {
  // eax <- nr
  // esi <- a
  // edi <- b
  // out 0xFF, eax
  __asm__ volatile(
      "mov %1, %%eax;"
      "mov %2, %%edi;"
      "mov %3, %%esi;"
      "out %%eax, $0xFF;"
      "mov %%eax, %0;"
      : "=r"(nr)
      : "r"(nr), "r"(a), "r"(b)
      : "eax", "edi", "esi");
  return nr;
}

char content_buf[600];
char recv_buf[500];
void kmain(void) {
  // IN MAIN
  record_timestamp();
  int nrecv = hcall(0, (unsigned long)recv_buf, 500);

  // AFTER RECV
  record_timestamp();

  int written = 0;
  char *dst = content_buf;
  *dst = nrecv;
  dst = append_string(dst, "HTTP/1.1 200 OK\r\n");
  dst = append_string(dst, "Content-Length: 0\r\n");

  hcall(1, (unsigned long)content_buf, (int)(dst - content_buf));

  // AFTER SEND
  record_timestamp();
  return;
}
