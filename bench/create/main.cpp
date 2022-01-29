#include <stdio.h>
#include <wasp/Virtine.h>
#include <memory>
#include <bench.h>
#include <wasp/Cache.h>


#define NPOINTS 1000
long data[NPOINTS];

// measures the latency to allocate a virtine context in cycles
int main(int argc, char **argv) {

  for (int i = 0; i < NPOINTS; i++) {
    auto start = wasp::tsc();
    wasp::Virtine v;
    v.allocate_memory(4096);
    v.load_raw(MINIMAL_VIRTINE, MINIMAL_VIRTINE_SIZE, 0);
    // run until any exit
    v.run();
    auto end = wasp::tsc();
		data[i] = end - start;
  }


  printf("# trial, latency (cycles)\n");
  for (int i = 0; i < NPOINTS; i++) {
    printf("%d, %lu\n", i, data[i]);
  }
}
