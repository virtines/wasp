#include <stdio.h>
#include <wasp/Virtine.h>
#include <memory>
#include <bench.h>
#include <wasp/Cache.h>

#define round_up(x, y) (((x) + (y)-1) & ~((y)-1))


int main(int argc, char **argv) {
  FILE *stream = fopen("build/echo_server.bin", "r");
  if (stream == NULL) return -1;

  fseek(stream, 0, SEEK_END);
  size_t sz = ftell(stream);
  void *bin = malloc(sz);
  fseek(stream, 0, SEEK_SET);
  // printf("mem: %p\n", mem);
  fread(bin, sz, 1, stream);
  fclose(stream);

  printf("PAE,kmain(),after recv(),after send()\n");
    wasp::Virtine v;
    v.allocate_memory(0x4000 + round_up(sz, 4096));
  for (int i = 0; i < 10000; i++) {
    v.load_raw(bin, sz, 0x4000);
    auto start = wasp::tsc();

    bool running = true;
    int iter = 0;
    while (running) {
      // run until any exit
      auto reason = v.run();
      if (reason == wasp::ExitReason::HyperCall) {
        auto regs = v.read_regs();
        auto hcall = regs.rax;
				// printf("hcall %llu %llu %llu\n", hcall, regs.rdi, regs.rsi);
				v.write_regs(regs);

        switch (hcall) {
          // recv
          case 0:
            // printf("recv(%llx, %llu)\n", regs.rdi, regs.rsi);
            break;
          case 1:
            // printf("send(%llx, %llu)\n", regs.rdi, regs.rsi);
            break;
          default:
            break;
        }

      } else {
				uint64_t *tsc = v.translate<uint64_t>(0);
				auto baseline = tsc[0];
				for (int i = 1; tsc[i] != 0; i++) {
					if (i != 1) printf(",");
					printf("%lu", tsc[i] - baseline);
				}
				printf("\n");
        running = false;
      }
    }
    auto end = wasp::tsc();
    // printf("%d, %lu\n", i, end - start);
    v.reset();
  }
}
