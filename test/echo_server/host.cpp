#include <stdio.h>
#include <wasp/Virtine.h>
#include <memory>
#include <bench.h>
#include <wasp/Cache.h>
#include <wasp/util.h>
#include <sys/wait.h>
#include <strings.h>

#include <curl/curl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define round_up(x, y) (((x) + (y)-1) & ~((y)-1))

/*
 * error - wrapper for perror used for bad syscalls
 */
void error(const char *msg) {
  perror(msg);
  exit(1);
}


int run_http_clients(int iters) {
  // initialize all of CURL
  curl_global_init(CURL_GLOBAL_DEFAULT);
  for (int i = 0; i < iters; i++) {
    CURL *curl = curl_easy_init();
    if (curl) {
      CURLcode res;
      curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000");
      res = curl_easy_perform(curl);
      if (res != 0) {
        fprintf(stderr, "res=%d %s\n", res, curl_easy_strerror(res));
      }
      curl_easy_cleanup(curl);
    }
  }
  return 0;
}

int run_virtines(int iters) {
  socklen_t clientlen;           /* byte size of client's address */
  struct hostent *hostp;         /* client host info */
  char *hostaddrp;               /* dotted decimal host addr string */
  int optval;                    /* flag value for setsockopt */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */

  // start the server
  /* open socket descriptor */
  int serverfd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverfd < 0) {
    error("ERROR opening socket");
  }

  /* allows us to restart server immediately */
  optval = 1;
  setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

  /* bind port to socket */
  bzero((char *)&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)8000);
  if (bind(serverfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
    error("ERROR on binding");
  }


  /* get us ready to accept connection requests */
  if (listen(serverfd, 1) < 0) /* allow 5 requests to queue up */
    error("ERROR on listen");

  // prep the virtine state by loading the binary.
  FILE *stream = fopen("build/echo_server.bin", "r");
  if (stream == NULL) return -1;

  fseek(stream, 0, SEEK_END);
  size_t sz = ftell(stream);
  void *bin = malloc(sz);
  fseek(stream, 0, SEEK_SET);
  // printf("mem: %p\n", mem);
  fread(bin, sz, 1, stream);
  fclose(stream);


  // now that we have the server, fork!
  pid_t pid = fork();
  if (pid == 0) {
    run_http_clients(iters);
    exit(0);
  }

  printf("PAE,kmain(),after recv(),after send()\n");
  for (int i = 0; i < iters; i++) {
		// *create* a virtine for each connection (don't reuse or cache)
    wasp::Virtine v;
    v.allocate_memory(0x4000 + round_up(sz, 4096));
    v.load_raw(bin, sz, 0x4000);

    /* wait for a connection request */
    int connectionfd = accept(serverfd, (struct sockaddr *)&clientaddr, &clientlen);
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
            regs.rax = recv(connectionfd, v.translate<void>(regs.rdi), regs.rsi, 0);
            // printf("recv(%llx, %llu)\n", regs.rdi, regs.rsi);
            break;
          case 1:
            // wasp::hexdump(v.translate<void>(regs.rdi), regs.rsi);
            regs.rax = send(connectionfd, v.translate<void>(regs.rdi), regs.rsi, 0);
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
    v.reset();

    close(connectionfd);
  }

  close(serverfd);
  return pid;
}


int main(int argc, char **argv) {
  int iters = 1000;

  pid_t child = run_virtines(iters);
  waitpid(child, NULL, 0);
  return 0;
}
