
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

#define JSINTERP_PATH "/home/cc/wasp/build/jsinterp.bin"



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


static bool use_snapshots = true;
static bool do_teardown = true;
static bool quiet = false;


class VirtineJSEngine {
  wasp::Cache cache;

 public:
  VirtineJSEngine() : cache(1024 * 1024 * 1) {
    FILE *stream = fopen(JSINTERP_PATH, "r");
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
    while (!done) {
      int ex = v->run();
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

static VirtineJSEngine engine;


extern "C" char* js_x(char* code1) {
  std::string code = std::string(code1);
  std::string ret = engine.evaluate(code);
  return (char*)ret.c_str();
}

extern "C" void get_cache_stats(int *misses, int *hits) {
  *misses = engine.cache.misses();
  *hits = engine.cache.hits();
}

extern "C" void foo(){
  printf("inside foo()\n");
}
