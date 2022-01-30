/*
 * This file is part of the Wasp hypervisor developed at Illinois Institute of
 * Technology (HExSA Lab) and Northwestern University with funding from the
 * United States National Science Foundation.
 *
 * Copyright (c) 2022, Nicholas Wanninger
 *
 * Author:  Nicholas Wanninger <ncw@u.northwestern.edu>
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "LICENSE.txt".
 */
#include <fcntl.h>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <wasp/Cache.h>
#include <wasp/Virtine.h>
#include <wasp/util.h>
#include <wasp/binding.h>
// for struct virtine_config
#include "virtine.h"

static std::map<size_t, std::unique_ptr<wasp::Cache>> cached_virtines;
std::mutex virtine_cache_lock;

static inline wasp::Cache &get_cache(const char *code, size_t codesz, size_t memsz) {
  auto found = cached_virtines.find(memsz);
  if (found == cached_virtines.end()) {
    auto c = std::make_unique<wasp::Cache>(memsz);
    c->set_binary(code, codesz, 0x8000);
    cached_virtines[memsz] = move(c);
    return *cached_virtines[memsz];
  }

  return *(*found).second;
}

bool hypercall_allowed(int nr, uint64_t hypercall_whitelist) {
  if (nr == 0xFF) return true;

  return (hypercall_whitelist >> nr) & 1;
}



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

void dump_trapframe(wasp::VirtineRegisters *r) {
#define GET(name) (r->name)
#define dump(reg) printf("%3s=%016llx ", #reg, GET(reg))
  dump(rax);
  dump(rbx);
  dump(rcx);
  dump(rdx);
  printf("\n");
  dump(rsi);
  dump(rdi);
  dump(rbp);
  dump(rsp);
  printf("\n");
  dump(r8);
  dump(r9);
  dump(r10);
  dump(r11);
  printf("\n");
  dump(r12);
  dump(r13);
  dump(r14);
  dump(r15);
  printf("\n");
  dump(rip);
  printf("flg=%016llx ", r->rflags);
  printf(" [%c%c%c%c%c%c%c]\n", r->rflags & DF_MASK ? 'D' : '-', r->rflags & CC_O ? 'O' : '-', r->rflags & CC_S ? 'S' : '-',
      r->rflags & CC_Z ? 'Z' : '-', r->rflags & CC_A ? 'A' : '-', r->rflags & CC_P ? 'P' : '-', r->rflags & CC_C ? 'C' : '-');
#undef dump
}

extern "C" void wasp_run_virtine(const char *code, size_t codesz, size_t memsz, void *arg, size_t argsz, void *vconfig) {
  // if WASP_NO_SNAPSHOT is not set, use snapshots
  static int g_use_snapshots = getenv("WASP_NO_SNAPSHOT") == NULL;

  int use_snapshots = g_use_snapshots;

  // allow all unless told otherwise
  uint64_t hypercall_whitelist = VIRTINE_ALLOW_ALL;
  struct virtine_config *config = (struct virtine_config *)vconfig;

  if (vconfig != NULL) {
    hypercall_whitelist = config->hypercall_whitelist;
  }

  // printf("run with size %zu, code: %zu\n", memsz, codesz);
  virtine_cache_lock.lock();
  auto &cache = get_cache(code, codesz, memsz);
  virtine_cache_lock.unlock();

  auto vm = cache.get();

  void *arg_pos = vm->translate<void>(0);
  if (arg != NULL) {
    // copy the argument into the machine's ram
    memcpy(arg_pos, arg, argsz);
  }

  bool done = false;
  if (!use_snapshots) {
    auto regs = vm->read_regs();
    regs.rip = regs.rsp = 0x8000;
    vm->write_regs(regs);
  }

  int count = 0;
  while (!done) {
    int res = vm->run();

    count++;

    if (res == wasp::ExitReason::HyperCall) {
      auto regs = vm->read_regs();
      int nr = regs.rdi;
      long long arg1 = regs.rsi;
      long long arg2 = regs.rdx;
      long long arg3 = regs.rcx;
      void *ptr = 0;
#define TRANSLATE(type, addr) (type)((off_t)ram + (addr))
      if (hypercall_allowed(nr, hypercall_whitelist)) {
        switch (regs.rdi) {
          case 0xFF: {
            if (use_snapshots) {
              regs.rip += 2;  // skip over the out instruction
              vm->write_regs(regs);
              virtine_cache_lock.lock();
              auto &cache = get_cache(code, codesz, memsz);
              virtine_cache_lock.unlock();
              std::vector<wasp::ResetMemory> regions;
              arg1 = 0;
              wasp::ResetMemory m;
              m.size = arg2 - arg1;
              m.data = malloc(m.size);
              m.address = arg1;
              memcpy(m.data, vm->translate<void>(m.address), m.size);
              regions.push_back(std::move(m));

              vm->save_reset_state(std::move(regions));
            }
            break;
          }
          case HCALL_exit:
            done = true;
            break;

          case HCALL_close:
            regs.rax = close(arg1);
            break;

          case HCALL_open:
            if (vm->validate_length(arg1, 256)) {
              regs.rax = open(vm->translate<const char>(arg1), arg2, arg3);
            }

            break;

          case HCALL_read:
            if (vm->validate_length(arg2, arg3)) {
              regs.rax = read(arg1, vm->translate<void>(arg2), arg3);
            }
            break;

          case HCALL_write:
            if (vm->validate_length(arg2, arg3)) {
              ptr = vm->translate<void>(arg2);
              regs.rax = write(arg1, ptr, arg3);
            }
            break;

          case HCALL_fstat: {
            if (vm->validate_length(arg2, sizeof(struct stat))) {
              regs.rax = fstat(arg1, vm->translate<struct stat>(arg2));
            }
            break;
          }
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
      } else {
        fprintf(stderr, "Virtine tried to use non-whitelisted hypercall %d\n", nr);
        fprintf(stderr, "    Permitted hypercalls:");
        for (int i = 0; i < sizeof(uint64_t) * 8; i++)
          if (hypercall_allowed(i, hypercall_whitelist)) fprintf(stderr, " %d", i);
        fprintf(stderr, "\n");
        regs.rax = -ENOSYS;
      }
    }

    if (res == wasp::ExitReason::Crashed) {
      fprintf(stderr, "[Virtine] FATAL - CRASHED.\n");
      auto regs = vm->read_regs();
      dump_trapframe(&regs);
			abort();
    }

    if (res == wasp::ExitReason::Exited) {
      done = true;
    }
  }

  // copy the argument out of the machine's ram
  if (arg != NULL) memcpy(arg, arg_pos, argsz);

  virtine_cache_lock.lock();
  get_cache(code, codesz, memsz).put(vm);
  virtine_cache_lock.unlock();
}










#define C_RED 91
#define C_GREEN 92
#define C_YELLOW 93
#define C_BLUE 94
#define C_MAGENTA 95
#define C_CYAN 96

#define C_RESET 0
#define C_GRAY 90

static void set_color(int code) {
  static int current_color = 0;
  if (code != current_color) {
    printf("\x1b[%dm", code);
    current_color = code;
  }
}

static void set_color_for(char c) {
  if (c >= 'A' && c <= 'z') {
    set_color(C_YELLOW);
  } else if (c >= '!' && c <= '~') {
    set_color(C_CYAN);
  } else if (c == '\n' || c == '\r') {
    set_color(C_GREEN);
  } else if (c == '\a' || c == '\b' || c == 0x1b || c == '\f' || c == '\n' || c == '\r') {
    set_color(C_RED);
  } else if ((unsigned char)c == 0xFF) {
    set_color(C_MAGENTA);
  } else {
    set_color(C_GRAY);
  }
}


void wasp::hexdump(void *vbuf, size_t len) {
  unsigned awidth = 4;

  if (len > 0xFFFFL) awidth = 8;

  unsigned char *buf = (unsigned char *)vbuf;
  int w = 16;

  // array of valid address checks
  char valid[16];

  int has_validated = 0;
  off_t last_validated_page = 0;
  int is_valid = 0;

  for (unsigned long long i = 0; i < len; i += w) {
    unsigned char *line = buf + i;


    for (int c = 0; c < w; c++) {
      off_t page = (off_t)(line + c) >> 12;

      if (!has_validated || page != last_validated_page) {
        is_valid = 1;
        has_validated = 1;
      }

      valid[c] = is_valid;
      last_validated_page = page;
    }

    set_color(C_RESET);
    printf("|");
    set_color(C_GRAY);

    printf("%.*llx", awidth, i);

    set_color(C_RESET);
    printf("|");
    for (int c = 0; c < w; c++) {
      if (c % 8 == 0) {
        printf(" ");
      }

      if (valid[c] == 0) {
        set_color(C_RED);
        printf("?? ");
        continue;
      }

      if (i + c >= len) {
        printf("   ");
      } else {
        set_color_for(line[c]);
        printf("%02X ", line[c]);
      }
    }

    set_color(C_RESET);
    printf("|");
    for (int c = 0; c < w; c++) {
      if (c != 0 && (c % 8 == 0)) {
        set_color(C_RESET);
        printf(" ");
      }


      if (valid[c] == 0) {
        set_color(C_RED);
        printf("?");
        continue;
      }

      if (i + c >= len) {
        printf(" ");
      } else {
        set_color_for(line[c]);
        printf("%c", (line[c] < 0x20) || (line[c] > 0x7e) ? '.' : line[c]);
      }
    }
    set_color(C_RESET);
    printf("|\n");
  }
}
