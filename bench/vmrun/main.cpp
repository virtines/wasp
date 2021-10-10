#include <bench.h>
#include <memory>
#include <stdio.h>
#include <wasp/Virtine.h>

//  measures the cost to run a null virtine, minus the cost of
// virtine allocation (the virtine is reused)

int main(int argc, char **argv) {
  printf("# trial, latency (cycles)\n");
  wasp::Virtine virtine;
  virtine.allocate_memory(4096);
  for (int i = 0; i < 1000; i++) {
    auto start = wasp::tsc();

    virtine.load_raw(MINIMAL_VIRTINE, 1, 0);
    int ex = virtine.run();

    auto end = wasp::tsc();
    virtine.reset();

    printf("%d, %lu\n", i, end - start);
  }
}
