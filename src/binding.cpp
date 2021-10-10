#include <fcntl.h>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <wasp/Cache.h>
#include <wasp/Virtine.h>
#include <wasp/binding.h>
// for struct virtine_config
#include "virtine.h"

static std::map<size_t, std::unique_ptr<wasp::Cache>> cached_virtines;
std::mutex virtine_cache_lock;

static inline wasp::Cache &get_cache(const char *code, size_t codesz,
                                     size_t memsz) {
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
  if (nr == 0xFF)
    return true;

  return (hypercall_whitelist >> nr) & 1;
}

extern "C" void wasp_run_virtine(const char *code, size_t codesz, size_t memsz,
                                 void *arg, size_t argsz, void *vconfig) {
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
#if 0
    auto regs = vm->read_regs();
    printf("> rip=%08llx, rsp=%08llx\n", regs.rip, regs.rsp);
#endif
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
            regs.rip += 2; // skip over the out instruction
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
            // printf("save region 0x%08lx-%08lx, rip=0x%08llx\n", m.address,
            // m.address + m.size, regs.rip);

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
          // printf("fstat(%d) = %d\n", arg1, regs.rax);
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
#if 0
				fprintf(stderr, "Virtine tried to use non-whitelisted hypercall %d\n", nr);
				fprintf(stderr, "    Permitted hypercalls:");
				for (int i = 0; i < sizeof(uint64_t) * 8; i++) {
					if (hypercall_allowed(i, hypercall_whitelist))
						fprintf(stderr, " %d", i);
				}

				fprintf(stderr, "\n");
#endif

        regs.rax = -ENOSYS;
      }
    }

    if (res == wasp::ExitReason::Crashed) {
      exit(-1);
    }

    if (res == wasp::ExitReason::Exited) {
      done = true;
    }
  }

  // copy the argument out of the machine's ram
  if (arg != NULL)
    memcpy(arg, arg_pos, argsz);

  virtine_cache_lock.lock();
  get_cache(code, codesz, memsz).put(vm);
  virtine_cache_lock.unlock();
}
