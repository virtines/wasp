#include <chrono>
#include <thread>
#include "time.h"

/**
 * Simple function to test creation/destruction latency
 */
int the_func(void) { return 42; }

int main(void) {
  for (int i = 0; i < 1000; i++) {
    auto start = rdtsc();

    auto thd = std::thread([]() { the_func(); });

    thd.join();
    auto dur = rdtsc() - start;
    printf("%d,%ld\n", i, dur);
  }
}
