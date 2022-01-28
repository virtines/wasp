#include <bench.h>
#include <memory>
#include <stdio.h>
#include <wasp/Virtine.h>
#include <sys/wait.h>
#include <pthread.h>

//  measures the cost to spawn, run, and join a null pthread




int main(int argc, char **argv) {
  printf("# trial, latency (cycles)\n");
  for (int i = 0; i < 1000; i++) {
    auto start = wasp::tsc();

    pthread_t thd;

		pid_t pid = fork();
		if (pid == 0) {
			exit(0);
		}

		waitpid(pid, NULL, 0);

    auto end = wasp::tsc();

    printf("%d, %lu\n", i, end - start);
  }
}
