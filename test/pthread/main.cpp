#include <bench.h>
#include <memory>
#include <stdio.h>
#include <wasp/Virtine.h>
#include <pthread.h>

//  measures the cost to spawn, run, and join a null pthread


void *thread(void *) { return NULL; }


int main(int argc, char **argv) {
  printf("# trial, latency (cycles)\n");
  for (int i = 0; i < 1000; i++) {
    auto start = wasp::tsc();

    pthread_t thd;

    pthread_create(&thd, NULL, thread, NULL);
    pthread_join(thd, NULL);

    auto end = wasp::tsc();

    printf("%d, %lu\n", i, end - start);
  }
}
