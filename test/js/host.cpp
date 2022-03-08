
#include <stdio.h>
#include <wasp/Virtine.h>
#include <wasp/util.h>
#include <memory>
#include <string.h>
#include "rt/virtine.h"
#include <wasp/Cache.h>
#include <fcntl.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>  //std::stringstream
#include <pthread.h>
#include <sched.h>
#define TEST_PATH "/home/cc/wasp/Build/jsinterp.bin"



/* eflags masks */
#define CC_C 0x0001
#define CC_P 0x0004
#define CC_A 0x0010
#define CC_Z 0x0040
#define CC_S 0x0080
#define CC_O 0x0800

#define TF_SHIFT 8
#define IOPL_SHIFT 12
#define VM_SHIFT 17
#define TF_MASK 0x00000100
#define IF_MASK 0x00000200
#define DF_MASK 0x00000400
#define IOPL_MASK 0x00003000
#define NT_MASK 0x00004000
#define RF_MASK 0x00010000
#define VM_MASK 0x00020000
#define AC_MASK 0x00040000
#define VIF_MASK 0x00080000
#define VIP_MASK 0x00100000
#define ID_MASK 0x00200000
void dump_regs(wasp::VirtineRegisters *tf) {
  // scoped_lock l(trapframe_dump_lock);
  unsigned int eflags = tf->rflags;
#define GET(name) (tf->name)
#define REGFMT "%016llx"
  printf("RAX=" REGFMT " RBX=" REGFMT " RCX=" REGFMT " RDX=" REGFMT
         "\n"
         "RSI=" REGFMT " RDI=" REGFMT " RBP=" REGFMT " RSP=" REGFMT
         "\n"
         "R8 =" REGFMT " R9 =" REGFMT " R10=" REGFMT " R11=" REGFMT
         "\n"
         "R12=" REGFMT " R13=" REGFMT " R14=" REGFMT " R15=" REGFMT
         "\n"
         "RIP=" REGFMT " RFL=%08x [%c%c%c%c%c%c%c]\n",

      GET(rax), GET(rbx), GET(rcx), GET(rdx), GET(rsi), GET(rdi), GET(rbp), GET(rsp), GET(r8), GET(r9), GET(r10), GET(r11), GET(r12),
      GET(r13), GET(r14), GET(r15), GET(rip), eflags, eflags & DF_MASK ? 'D' : '-', eflags & CC_O ? 'O' : '-', eflags & CC_S ? 'S' : '-',
      eflags & CC_Z ? 'Z' : '-', eflags & CC_A ? 'A' : '-', eflags & CC_P ? 'P' : '-', eflags & CC_C ? 'C' : '-');
}


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


static bool use_snapshots = false;
static bool do_teardown = false;
static bool quiet = false;


class VirtineJSEngine {
  wasp::Cache cache;

 public:
  VirtineJSEngine() : cache(1024 * 1024 * 1) {
    FILE *stream = fopen("/home/cc/wasp/build/jsinterp.bin", "r");
    if (stream == NULL) abort();

    fseek(stream, 0, SEEK_END);
    size_t sz = ftell(stream);
    void *bin = malloc(sz);
    fseek(stream, 0, SEEK_SET);
    // printf("mem: %p\n", mem);
    fread(bin, sz, 1, stream);
    fclose(stream);

    cache.set_binary(bin, sz, 0x8000);
  }


  std::string evaluate(const std::string &code) {
    void *argument = (void *)code.data();
    size_t argsize = code.size();

    wasp::Virtine *v = cache.get();

    bool done = false;
    std::string result;

    *v->translate<int>(0x1000) = do_teardown;
    int i = 0;
    // printf("==\n");
    while (!done) {
    auto start = wasp::time_us();
      int ex = v->run();
      auto end = wasp::time_us();
      if (true || i == 0) {
          // printf("%lu us\n", end - start);
      }
      i++;
      if (ex == wasp::ExitReason::Crashed) {
				printf("crash\n");
				break;
			}
      if (ex == wasp::ExitReason::Exited) {
				printf("exit\n");
        break;
      }
      if (ex == wasp::ExitReason::HyperCall) {
        auto regs = v->read_regs();

        int nr = regs.rdi;
        long long arg1 = regs.rsi;
        long long arg2 = regs.rdx;
        long long arg3 = regs.rcx;
        void *ptr = 0;

        if (regs.rdi == 0xFF) {
          if (use_snapshots) {
            regs.rip += 2;  // skip over the out instruction
            v->write_regs(regs);
            std::vector<wasp::ResetMemory> regions;
            wasp::ResetMemory m;
            m.size = arg2 - arg1;
            m.data = malloc(m.size);
            m.address = arg1;
            memcpy(m.data, v->translate<void>(m.address), m.size);
            regions.push_back(std::move(m));
            v->save_reset_state(std::move(regions));
          }
          continue;
        }
        switch (regs.rdi) {
          case HCALL_exit:
            done = true;
            break;

            // allow reading from file descriptor 0
          case HCALL_read:
            if (arg1 != 0) {
              regs.rax = -EPERM;
            } else {
              regs.rax = read(arg1, v->translate<void>(arg2), arg3);
            }
            break;

          case HCALL_write:
            if (arg1 != 1 && arg1 != 2) {
              regs.rax = -EPERM;
            } else {
              ptr = v->translate<void>(arg2);
              regs.rax = write(arg1, ptr, arg3);
            }
            break;

          case HCALL_GET_ARG: {
            ptr = v->translate<void>(arg1);
            size_t size = arg2;
            if (arg1 == 0) {
              regs.rax = argsize;
              break;
            } else {
              if (size >= argsize) {
                memcpy(ptr, argument, argsize);
                regs.rax = argsize;
                break;
              }
            }
            break;
          }

          case HCALL_RETURN: {
            done = true;
            char *ptr = v->translate<char>(arg1);
            size_t size = arg2;
            result = std::string(ptr, size);
            break;
          }

          case HCALL_open:
          case HCALL_close:
          case HCALL_fstat:
          case HCALL_sbrk:
          case HCALL_link:
          case HCALL_lseek:
          case HCALL_times:
          case HCALL_unlink:
          case HCALL_gettimeofday:
          case HCALL_isatty:
          default:
            regs.rax = -ENOSYS;
            break;
        }

        if (!done) v->write_regs(regs);
      }
    }
    cache.put(v);
    return result;
  }
};
VirtineJSEngine engine;


std::string code;
void *worker(void *p) {
  long cpu = (long)p * 2;
  cpu_set_t my_set;                                 /* Define your cpu_set bit mask. */
  CPU_ZERO(&my_set);                                /* Initialize it all to 0, i.e. no CPUs selected. */
  CPU_SET(cpu, &my_set);                            /* set the bit that represents core 7. */
  sched_setaffinity(0, sizeof(cpu_set_t), &my_set); /* Set affinity of tihs process to */
  sched_yield();
  
  for (int i = 0; true; i++) {
    auto start = wasp::time_us();
    engine.evaluate(code);
    auto end = wasp::time_us();
    // printf("%d, %d,%lu\n", gettid(), i, end - start);
  }
  return NULL;
}

int main(int argc, char **argv) {
  int opt;
  while ((opt = getopt(argc, argv, "qst")) != -1) {
    switch (opt) {
      case 'q':
        quiet = true;
        break;
      case 's':
        use_snapshots = true;
        break;
      case 't':
        do_teardown = true;
        break;
    }
  }

  if (optind > argc) {
    fprintf(stderr, "usage: js [st] file.js\n");
    exit(EXIT_FAILURE);
  }


  void *argument = 0;
  size_t argsize = 0;

  std::ifstream t(argv[optind]);
  std::stringstream buffer;
  buffer << t.rdbuf();
  code = buffer.str();  // str holds the content of the file


  printf("# trial, latency\n");

  #define NTHREAD 24

  pthread_t threads[NTHREAD];


  for (int i = 0; i < NTHREAD; i++) 
    pthread_create(&threads[i], NULL, worker, (void*)i);
  for (int i = 0; i < NTHREAD; i++) 
    pthread_join(threads[i], NULL);



}
