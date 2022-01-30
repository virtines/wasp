#include <stdio.h>
#include <wasp/Virtine.h>
#include <memory>
#include <bench.h>
#include <wasp/Cache.h>


#define round_up(x, y) (((x) + (y)-1) & ~((y)-1))
#define NPOINTS 1000

int main(int argc, char **argv) {
  FILE *stream = fopen("build/boottime.virtine", "r");
  if (stream == NULL) return -1;

  fseek(stream, 0, SEEK_END);
  size_t sz = ftell(stream);
  void *bin = malloc(sz);
  fseek(stream, 0, SEEK_SET);
  fread(bin, sz, 1, stream);
  fclose(stream);


  printf(
      "# trial, First Instruction, Load 32-bit GDT, Protected Mode Enable, Protected Transition, Paging identity mapping, Load 64-bit GDT, "
      "Jump to 64-bit\n");

  wasp::Virtine v;
  v.allocate_memory(0x8000 + round_up(sz, 4096));
  for (int i = 0; i < NPOINTS; i++) {
    v.load_raw(bin, sz, 0x8000);

    // run until any exit
    auto start = wasp::tsc();
    auto reson = v.run();
    auto end = wasp::tsc();

    printf("%d", i);

    auto *tsc = v.translate<uint64_t>(0);
    for (int i = 1; i < 8; i++) {
      auto prev = tsc[i - 1];
      auto curr = tsc[i];

      printf(", %lu", curr - prev);
    }
    printf("\n");

    v.reset();
  }

  return 0;
}
