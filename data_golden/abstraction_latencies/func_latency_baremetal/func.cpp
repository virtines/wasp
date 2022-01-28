#include <chrono>
#include <thread>
#include "time.h"


/**
 * Simple function to test creation/destruction latency
 */
int the_func(void) {
  return 42;
}

int main(void) {
  for (int i = 0; i < 1000; i++) {
    auto start = rdtsc();
    the_func();

    auto dur = rdtsc() - start;
    printf("%d,%zu\n", i, dur);
  }
}
