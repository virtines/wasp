#include <stdio.h>
#include <wasp/Virtine.h>
#include <memory>
#include <bench.h>
#include <wasp/Cache.h>

#define round_up(x, y) (((x) + (y)-1) & ~((y)-1))


// a cache where each virtine has 4k of memory

int main(int argc, char **argv) {
  FILE *stream = fopen(argv[1], "r");
  if (stream == NULL) return -1;

  fseek(stream, 0, SEEK_END);
  size_t sz = ftell(stream);
  void *bin = malloc(sz);
  fseek(stream, 0, SEEK_SET);
  // printf("mem: %p\n", mem);
  fread(bin, sz, 1, stream);
  fclose(stream);


  wasp::Cache virtine_cache(4096 * 32);
  virtine_cache.set_binary(bin, sz, 0x8000);

  printf("# trial, latency (cycles)\n");

  wasp::Virtine v;
  v.allocate_memory(0x8000 + round_up(sz, 4096));
  for (int i = 0; i < 100000; i++) {
    v.load_raw(bin, sz, 0x8000);
    auto start = wasp::tsc();
    // run until any exit
    v.run();
    auto end = wasp::tsc();
    printf("%d, %lu\n", i, end - start);
    v.reset();
  }
}
