#include <stdio.h>
#include <wasp/Virtine.h>
#include <wasp/Cache.h>
#include <memory>
#include <assert.h>
#include <wasp/util.h>

#define TEST_PATH "build/tests/cache.bin"
#define MEMORY_SIZE (4096 * 2)

wasp::Cache virtine_cache(MEMORY_SIZE);

int main(int argc, char **argv) {
  FILE *stream = fopen(TEST_PATH, "r");

  fseek(stream, 0, SEEK_END);
  size_t sz = ftell(stream);
  fseek(stream, 0, SEEK_SET);
  void *bin = malloc(sz);
  fread(bin, sz, 1, stream);
  fclose(stream);

	virtine_cache.set_binary(bin, sz, 0);

  auto start = wasp::time_us();
  virtine_cache.ensure(4);


  printf("ensure took %luus\n", wasp::time_us() - start);

  for (int i = 0; i < 300; i++) {
    auto start = wasp::time_us();
    auto *virtine = virtine_cache.get();

    assert(virtine != NULL);

    // virtine->load_binary(bin, sz, 0);

    while (1) {
      int ex = virtine->run();
			if (ex == wasp::ExitReason::HyperCall) {
				printf("hypercall!\n");
				continue;
			}

      if (ex == wasp::ExitReason::Exited || ex == wasp::ExitReason::HyperCall) {
				break;
      }

      if (ex == wasp::ExitReason::Crashed) {
        auto regs = virtine->read_regs();
        // virtine->dump();
        exit(-1);
      }
    }

    auto end = wasp::time_us();
    virtine_cache.put(virtine);

    printf("%d, %luus\n", i, end - start);
  }
}
