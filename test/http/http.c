#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>



#include "virtine.h"
#define BUFSIZE 1024
#define MAXERRS 16
#define MAX 4096
#define PORT 8080


#define LAT_TRIALS 1000
#define TPUT_NUM_TRIALS (100)
#define TPUT_SAMPLES_PER_TRIAL (1000)


#ifdef BASELINE
#undef virtine_whitelist
#define virtine_whitelist(...)
#endif

/*
 * error - wrapper for perror used for bad syscalls
 */
void error(const char *msg) {
  perror(msg);
  exit(1);
}

/*
 * cerror - returns an error message to the client
 */
void cerror(FILE *stream, const char *cause, const char *errno, const char *shortmsg, const char *longmsg) {
  fprintf(stream, "HTTP/1.1 %s %s\n", errno, shortmsg);
  fprintf(stream, "Content-type: text/html\n");
  fprintf(stream, "\n");
  fprintf(stream, "<html><title>Tiny Error</title>");
  fprintf(stream,
      "<body bgcolor="
      "ffffff"
      ">\n");
  fprintf(stream, "%s: %s\n", errno, shortmsg);
  fprintf(stream, "<p>%s: %s\n", longmsg, cause);
  fprintf(stream, "<hr><em>The Tiny Web server</em>\n");
}



char **str_split(char *a_str, const char a_delim) {
  char **result = 0;
  size_t count = 0;
  char *tmp = a_str;
  char *last_comma = 0;
  char delim[2];
  delim[0] = a_delim;
  delim[1] = 0;

  /* Count how many elements will be extracted. */
  while (*tmp) {
    if (a_delim == *tmp) {
      count++;
      last_comma = tmp;
    }
    tmp++;
  }

  /* Add space for trailing token. */
  count += last_comma < (a_str + strlen(a_str) - 1);

  /* Add space for terminating null string so caller
     knows where the list of returned strings ends. */
  count++;

  result = (char **)malloc(sizeof(char *) * count);

  if (result) {
    size_t idx = 0;
    char *token = strtok(a_str, delim);

    while (token) {
      *(result + idx++) = strdup(token);
      token = strtok(0, delim);
    }
    *(result + idx) = 0;
  }

  return result;
}


char res[BUFSIZE];      /* message buffer */
char filename[BUFSIZE]; /* path derived from uri */
char req[BUFSIZ];


virtine_whitelist(VIRTINE_ALLOW_OPEN | VIRTINE_ALLOW_CLOSE | VIRTINE_ALLOW_READ | VIRTINE_ALLOW_WRITE |
                  VIRTINE_ALLOW_FSTAT) int handle_connection(int client) {
  struct stat sbuf;

  read(client, req, BUFSIZE);

  char method[32];
  char version[32];


  const char *filetype = "text/html";
  const char *abspath = "test/http/index.html";

  int fd = open(abspath, O_RDONLY);

  // give a 404 if its not found
  if (fd < 0) {
    char *out = res;
    out += sprintf(out, "HTTP/1.1 404 Not Found\r\n\r\n");
    // write the http headers
    write(client, res, strlen(res));
    close(fd);
    return 0;
  }

  fstat(fd, &sbuf);


  // give a 404 if its not a valid file
  if (!S_ISREG(sbuf.st_mode)) {
    char *out = res;
    out += sprintf(out, "HTTP/1.1 404 Not Found\r\n\r\n");
    // write the http headers
    write(client, res, strlen(res));
    close(fd);
    return 0;
  }

  char *out = res;
  out += sprintf(out, "HTTP/1.1 200 OK\r\n");
  out += sprintf(out, "Content-length: %d\r\n", (int)sbuf.st_size);
  out += sprintf(out, "Content-type: %s\r\n", filetype);
  out += sprintf(out, "\r\n");
  // write the http headers
  write(client, res, strlen(res));


  while (1) {
    int nread = read(fd, res, BUFSIZE);
    if (nread <= 0) break;
    write(client, res, nread);
  }

  close(fd);
  return 0;
}




void handle(int sockfd) {
  char buf[MAX];
  size_t sz = snprintf(buf, MAX, "GET /index.html HTTP/1.1\r\n");
  write(sockfd, buf, sz);

  while (1) {
    ssize_t nread = read(sockfd, buf, MAX);
    if (nread <= 0) break;
  }
}

unsigned long read_timestamp(void) {
  uint32_t lo, hi;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return lo | ((uint64_t)(hi) << 32);
  uint64_t ret;
  asm volatile("pushfq; popq %0" : "=a"(ret));
  return ret;
}


uint64_t current_microsecond() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
}


int measure_throughput() {
  int sockfd, connfd;
  struct sockaddr_in servaddr;

  bzero(&servaddr, sizeof(servaddr));
  unsigned long real_start_us = current_microsecond();

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = htons(PORT);


  struct {
    unsigned long time;
  } trials[TPUT_NUM_TRIALS];

  printf("# trial, us for %d calls\n", TPUT_SAMPLES_PER_TRIAL);
  for (int t = 0; t < TPUT_NUM_TRIALS; t++) {
    unsigned long start = current_microsecond();
    for (int i = 0; i < TPUT_SAMPLES_PER_TRIAL; i++) {
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        close(sockfd);
        i--;
        continue;
      }
      handle(sockfd);
      close(sockfd);
    }
    trials[t].time = current_microsecond() - start;
    printf("%d, %lu\n", t, trials[t].time);
  }
  return 0;
}


void measure_latencies(void) {
  long trials[LAT_TRIALS];


  int sockfd, connfd;
  struct sockaddr_in servaddr;

  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = htons(PORT);


  printf("# trial, microseconds\n");

  for (int i = 0; i < LAT_TRIALS; i++) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
      close(sockfd);
      i--;
      continue;
    }

    unsigned long startus = current_microsecond();
    unsigned long start = read_timestamp();
    // function for chat
    handle(sockfd);
    unsigned long cycles = read_timestamp() - start;
    unsigned long now_us = current_microsecond();
    unsigned long micros = now_us - startus;
		trials[i] = micros;


    // close the socket
    close(sockfd);
  }
  for (int i = 0; i < LAT_TRIALS; i++) {
    printf("%d, %lu\n", i, trials[i]);
	}
}




void accept_one(int server) {
  /* wait for a connection request */
  struct sockaddr_in clientaddr; /* client addr */
  socklen_t clientlen = sizeof(clientaddr);
  int client = accept(server, (struct sockaddr *)&clientaddr, &clientlen);
  /* they close the connection fd */
  handle_connection(client);
  close(client);
}




int main(int argc, char **argv) {
  struct hostent *hostp;         /* client host info */
  char *hostaddrp;               /* dotted decimal host addr string */
  int optval;                    /* flag value for setsockopt */
  struct sockaddr_in serveraddr; /* server's addr */



  int opt;
	int get_tput = 0;
	int get_lat = 0;
  while ((opt = getopt(argc, argv, "tl")) != -1) {
    switch (opt) {
      case 't':
				get_tput = 1;
        break;
			case 'l':
				get_lat = 1;
				break;
    }
  }

  /* open socket descriptor */
  int server = socket(AF_INET, SOCK_STREAM, 0);
  if (server < 0) error("ERROR opening socket");

  /* allows us to restart server immediately */
  optval = 1;
  setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

  /* bind port to socket */
  bzero((char *)&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)PORT);
  if (bind(server, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) error("ERROR on binding");

  /* get us ready to accept connection requests */
  if (listen(server, 5) < 0) /* allow 5 requests to queue up */
    error("ERROR on listen");

	if (get_lat || get_tput) {


		int trials = LAT_TRIALS;
		if (get_tput) {
			trials = TPUT_NUM_TRIALS * TPUT_SAMPLES_PER_TRIAL;
		}


		pid_t pid = fork();
		if (pid == 0) {
			if (get_lat) measure_latencies();
			if (get_tput) measure_throughput();
			exit(0);
		}
		for (int i = 0; i < trials; i++) {
			accept_one(server);
		}
		waitpid(pid, NULL, 0);
		exit(0);
	}

  /*
   * main loop: wait for a connection request, parse HTTP,
   * serve requested content, close connection.
   */
  while (1) {
    accept_one(server);
  }
}
