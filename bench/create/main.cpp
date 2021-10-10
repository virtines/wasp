#include <stdio.h>
#include <wasp/Virtine.h>
#include <memory>
#include <bench.h>
#include <wasp/Cache.h>



// measures the latency to allocate a virtine context in cycles
int main(int argc, char **argv) {
  printf("# trial, latency (cycles)\n");

  for (int i = 0; i < 1000; i++) {
    auto start = wasp::tsc();
    wasp::Virtine v;
    v.allocate_memory(4096);
    v.load_raw(MINIMAL_VIRTINE, MINIMAL_VIRTINE_SIZE, 0);
    // run until any exit
    v.run();
    auto end = wasp::tsc();
    printf("%d, %lu\n", i, end - start);
  }
}
