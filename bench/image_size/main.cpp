#include <fcntl.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wasp/Cache.h>
#include <wasp/Virtine.h>
#include <wasp/binding.h>
#include <wasp/util.h>
#include <bench.h>

unsigned long nanos(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000 * 1000 * 1000 + ts.tv_nsec;
}



int main(int argc, char **argv) {
#define NPAGES 4096
  size_t max_size = NPAGES * 4096LLU;

  void *src = malloc(max_size);

  printf("# trial, bytes, microseconds\n");
  int count = 0;
  for (size_t bytes = 4096; bytes <= max_size; bytes <<= 1) {
    for (int j = 0; j < 100; j++) {
      count++;
      auto start = nanos();
      wasp::Virtine v;
      v.allocate_memory(bytes);
      void *dst = v.translate<void>(0);
      memcpy(dst, src, bytes);
      v.load_raw(MINIMAL_VIRTINE, MINIMAL_VIRTINE_SIZE, 0);
      v.run();
      unsigned long dur = nanos() - start;

      printf("%d, %zu, %f\n", j, bytes, dur / 1000.0f);
    }
  }
  printf("count %d\n", count);
}
