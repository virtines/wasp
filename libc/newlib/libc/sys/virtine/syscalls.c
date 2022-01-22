#include <stdarg.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>


char **environ; /* pointer to array of char * strings that define the current
                   environment variables */

#define HCALL_exit 0
#define HCALL_close 1
#define HCALL_fstat 2
#define HCALL_link 3
#define HCALL_lseek 4
#define HCALL_open 5
#define HCALL_read 6
#define HCALL_sbrk 7
#define HCALL_times 8
#define HCALL_unlink 9
#define HCALL_write 10
#define HCALL_gettimeofday 11
#define HCALL_isatty 12


#pragma alias "sscanf" = "__isoc99_sscanf"

long __syscall_ret(long r) {
  if (r < 0) {
    errno = -r;
    return -1;
  }
  errno = 0;
  return r;
}



extern long __hypercall(int nr, long long a, long long b, long long c);


void _exit() { __hypercall(HCALL_exit, 0, 0, 0); }
int _close(int file) { return __hypercall(HCALL_close, file, 0, 0); }
int close(int file) { return __hypercall(HCALL_close, file, 0, 0); }

int fstat(int file, struct stat *st) { return __hypercall(HCALL_fstat, file, (long long)st, 0); }
int _fstat(int file, struct stat *st) { return __hypercall(HCALL_fstat, file, (long long)st, 0); }


int _link(char *old, char *new_name) { return -ENOSYS; }
int _lseek(int file, int ptr, int dir) { return -ENOSYS; }



int open(const char *filename, int flags, ...) {
  mode_t mode = 0;

  if ((flags & O_CREAT)) {
    va_list ap;
    va_start(ap, flags);
    mode = va_arg(ap, mode_t);
    va_end(ap);
  }

  int fd = __hypercall(HCALL_open, (long long)filename, flags, mode);
  // if (fd>=0 && (flags & O_CLOEXEC)) {
  //  __syscall(SYS_fcntl, fd, F_SETFD, FD_CLOEXEC);
  // }


  return __syscall_ret(fd);
}



int read(int file, char *ptr, int len) { return __hypercall(HCALL_read, file, (long long)ptr, len); }


extern unsigned char __heap_start[];
static unsigned char *__eheap = (unsigned char *)(1024 * 1024 * 4);
static unsigned char *heapend = __heap_start;


off_t __heap_top() {
	return (off_t)heapend;
}


void *__sbrk(int incr) {
  if (heapend + incr > __eheap) {
    errno = ENOMEM;
    return (void *)-1;
  }

  unsigned char *newheapstart = heapend;
  heapend += incr;
  // __hypercall(HCALL_sbrk, incr, (long long)newheapstart, 0);
  return newheapstart;
}

caddr_t sbrk(int incr) {
  ssize_t ret = (size_t)__sbrk(incr);
  if (ret <= 0) {
    errno = ENOMEM;
    ret = 0;
  }
  return (caddr_t)ret;
}

int stat(const char *file, struct stat *st) {
  return __syscall_ret(__hypercall(HCALL_fstat, (long long)file, (long long)st, 0));
}


clock_t _times(struct tms *buf) { return -ENOSYS; }
int _unlink(char *name) { return -ENOSYS; }

int write(int file, char *ptr, int len) { return __hypercall(HCALL_write, file, (long long)ptr, len); }

int _gettimeofday(struct timeval *__restrict __p, void *__restrict __tz) {
  return __hypercall(HCALL_gettimeofday, (long long)__p, (long long)__tz, 0);
}

int _isatty(int file) { return __hypercall(HCALL_isatty, file, 0, 0); }

caddr_t _sbrk(int incr) { return sbrk(incr); }
int _write(int file, char *ptr, int len) { return write(file, ptr, len); };
int _read(int file, char *ptr, int len) { return read(file, ptr, len); };




// these systemcalls just don't work here...
int _execve(char *name, char **argv, char **env) { return -ENOSYS; }
int _fork() { return -ENOSYS; }
int _getpid() {
  // just return pid 1
  return 1;
}
int _kill(int pid, int sig) {
  // just not allowed. You are in a Virtine
  return -EPERM;
}
int _wait(int *status) {
  // process stuff just doesn't work.
  return -ENOSYS;
}



void _init(void) {}
void _fini(void) {}


void __snapshot(void) {
	int bottom = 0;
	// __hypercall(0xFF, (unsigned long)&bottom, __heap_top(), 0);
}
