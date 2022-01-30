
#include <fcntl.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wasp/Cache.h>
#include <wasp/Virtine.h>
#include <wasp/binding.h>
#include <wasp/util.h>

#define TEST_PATH "build/fib.bin"

const char argument[] = "HELLO THERE";

#pragma GCC push_options
#pragma GCC optimize("O0")

int __attribute__((noinline)) fib(int n) {
  if (n < 2)
    return n;
  // this is a hacky way to force gcc to not constant fold
  asm("");
  return fib(n - 1) + fib(n - 2);
}

#pragma GCC pop_options

unsigned long nanos(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);

  return ts.tv_sec * 1000 * 1000 * 1000 + ts.tv_nsec;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: fib <n>\n");
    return EXIT_FAILURE;
  }

  int n = atoi(argv[1]);
  int capture_baseline = getenv("GET_BASELINE") != NULL;

  FILE *stream = fopen(TEST_PATH, "r");
  if (stream == NULL)
    return -1;

  fseek(stream, 0, SEEK_END);
  size_t sz = ftell(stream);
  void *bin = malloc(sz);
  fseek(stream, 0, SEEK_SET);
  // printf("mem: %p\n", mem);
  fread(bin, sz, 1, stream);
  fclose(stream);

  // cache.ensure(12);
  printf("# n, latency\n");
  for (int i = 0; i < 1000; i++) {
    unsigned long start = nanos();
    if (capture_baseline) {
      fib(n);
    } else {
      wasp_run_virtine((const char *)bin, sz, 0x9000 + (sz & ~0xFFF), &n,
                       sizeof(n), NULL);
    }
    unsigned long end = nanos();
    printf("%d, %5.2f\n", n, (end - start) / 1000.0f);
  }
}
