#include <stdio.h>
#include <wasp/Virtine.h>
#include <memory>
#include <bench.h>
#include <wasp/Cache.h>


#define NPOINTS 1000
long data[NPOINTS];


int main(int argc, char **argv) {
  wasp::Cache virtine_cache(4096);

  virtine_cache.set_binary(MINIMAL_VIRTINE, MINIMAL_VIRTINE_SIZE, 0);

  printf("# trial, latency (cycles)\n");
  for (int i = 0; i < NPOINTS; i++) {
    auto start = wasp::tsc();
    auto *virtine = virtine_cache.get();

    // make sure the regs are setup right lol
    auto regs = virtine->read_regs();
    regs.rip = 0;
    virtine->write_regs(regs);
    // run until any exit
    virtine->run();
    virtine_cache.put(virtine);
    auto end = wasp::tsc();
		data[i] = end - start;
  }


  printf("# trial, latency (cycles)\n");
  for (int i = 0; i < NPOINTS; i++) {
    printf("%d, %lu\n", i, data[i]);
  }
}
