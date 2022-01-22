#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#ifdef __NAUTILUS__
#include <nautilus/nautilus.h>
#else
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef NORETURN
#define NORETURN	__attribute__((noreturn))
#endif

typedef unsigned int tid_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct sem;
typedef struct sem sem_t;

typedef void (*signal_handler_t)(int);

tid_t sys_getpid(void);
int sys_fork(void);
int sys_wait(int* status);
int sys_execve(const char* name, char * const * argv, char * const * env);
void NORETURN sys_exit(int arg);
ssize_t sys_read(int fd, char* buf, size_t len);
ssize_t sys_write(int fd, const char* buf, size_t len);
ssize_t sys_sbrk(ssize_t incr);
int sys_open(const char* name, int flags, int mode);
int sys_close(int fd);
int sys_clone(tid_t* id, void* ep, void* argv);
off_t sys_lseek(int fd, off_t offset, int whence);
void sys_yield(void);
int sys_kill(tid_t dest, int signum);
int sys_signal(signal_handler_t handler);
int sys_stat(const char * file, struct stat * st);

#define __NR_exit 		0
#define __NR_write		1
#define __NR_open		2
#define __NR_close		3
#define __NR_read		4
#define __NR_lseek		5
#define __NR_unlink		6
#define __NR_getpid		7
#define __NR_kill		8
#define __NR_fstat		9
#define __NR_sbrk		10
#define __NR_fork		11
#define __NR_wait		12
#define __NR_execve		13
#define __NR_times		14
#define __NR_stat		15
#define __NR_dup		16
#define __NR_dup2		17
#define __NR_msleep		18
#define __NR_yield		19
#define __NR_sem_init		20
#define __NR_sem_destroy	21
#define __NR_sem_wait		22
#define __NR_sem_post		23
#define __NR_sem_timedwait	24
#define __NR_getprio		25
#define __NR_setprio		26
#define __NR_clone		27
#define __NR_sem_cancelablewait	28
#define __NR_get_ticks		29

#ifdef __cplusplus
}
#endif

#endif
