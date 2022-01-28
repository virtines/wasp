#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include "time.h"

/**
 * Simple function to test creation/destruction latency
 */
int the_func(void) { return 42; }

int main(int argc, char **argv) {
  if (argc > 1) {
    return 0;
  }

  int status;

  if (argc == 1) {
    for (int i = 0; i < 1000; i++) {
      auto start = rdtsc();

      auto pid = fork();

      if (pid == 0) {
        // do the exec
        char *argv_list[] = {argv[0], (char *)"foo", NULL};
        execv(argv[0], argv_list);

        return 0;
      }

      if (waitpid(pid, &status, 0) > 0) {
        if (WIFEXITED(status) && !WEXITSTATUS(status)) {
          auto dur = rdtsc() - start;
          printf("%d,%ld\n", i, dur);
        }
      }
    }
  }
}
