#include <wasp/Virtine.h>
#include <wasp/Cache.h>
#include <wasp/util.h>
#include <bench.h>
#include <pthread.h>



wasp::Cache cache(4096);

void *worker(void *p) {
  cache.set_binary(MINIMAL_VIRTINE, MINIMAL_VIRTINE_SIZE, 0);
  int id = (long)p;
  for (int i = 0; i < 1000; i++) {
    auto start = wasp::time_us();
    auto v = cache.get();
    v->run();
    cache.put(v);
    auto end = wasp::time_us();
    // printf("%lu us\n", end - start);
  }
  return nullptr;
}

int main() {
#define NTHREAD 12
  pthread_t threads[NTHREAD];
  for (long i = 0; i < NTHREAD; i++)
    pthread_create(&threads[i], NULL, worker, (void *)i);
  for (long i = 0; i < NTHREAD; i++)
    pthread_join(threads[i], NULL);

  return EXIT_SUCCESS;
}
