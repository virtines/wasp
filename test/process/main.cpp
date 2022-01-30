#include <bench.h>
#include <memory>
#include <stdio.h>
#include <wasp/Virtine.h>
#include <sys/wait.h>
#include <pthread.h>

//  measures the cost to spawn, run, and join a null pthread




#define NPOINTS 1000
long data[NPOINTS];
int main(int argc, char **argv) {
  for (int i = 0; i < NPOINTS; i++) {
    auto start = wasp::tsc();

    pthread_t thd;

		pid_t pid = fork();
		if (pid == 0) {
			exit(0);
		}

		waitpid(pid, NULL, 0);

    auto end = wasp::tsc();

    data[i] = end - start;
  }


  printf("# trial, latency (cycles)\n");
  for (int i = 0; i < NPOINTS; i++) {
    printf("%d, %lu\n", i, data[i]);
  }
}
