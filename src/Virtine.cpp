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
#include <errno.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <math.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <wasp/Virtine.h>
#include <stdexcept>

struct cpuid_regs {
  int eax;
  int ebx;
  int ecx;
  int edx;
};
#define MAX_KVM_CPUID_ENTRIES 100
static void filter_cpuid(struct kvm_cpuid2 *);

// #define SANITY_CHECK


static int g_kvmfd = -1;

wasp::Virtine::Virtine() {
  if (g_kvmfd == -1) {
    g_kvmfd = open("/dev/kvm", O_RDWR);
    if (g_kvmfd == -1) throw std::runtime_error("unable to open /dev/kvm");
  }

  m_kvmfd = g_kvmfd;
  int ret = 0;

#ifdef SANITY_CHECK
  ret = ioctl(m_kvmfd, KVM_GET_API_VERSION, 0);
  if (ret == -1) throw std::runtime_error("KVM_GET_API_VERSION");
  if (ret != 12) throw std::runtime_error("KVM_GET_API_VERSION returned invalid version");

  ret = ioctl(m_kvmfd, KVM_CHECK_EXTENSION, KVM_CAP_USER_MEMORY);
  if (ret == -1) throw std::runtime_error("KVM_CHECK_EXTENSION");
  if (!ret) throw std::runtime_error("Required extension KVM_CAP_USER_MEM not available");
#endif

  // create a VM
  m_vmfd = ioctl(m_kvmfd, KVM_CREATE_VM, 0);

  int kvm_run_size = ioctl(m_kvmfd, KVM_GET_VCPU_MMAP_SIZE, nullptr);

  // get cpuid info
  struct kvm_cpuid2 *kvm_cpuid;

  kvm_cpuid = (kvm_cpuid2 *)calloc(1, sizeof(*kvm_cpuid) + MAX_KVM_CPUID_ENTRIES * sizeof(*kvm_cpuid->entries));
  kvm_cpuid->nent = MAX_KVM_CPUID_ENTRIES;
  if (ioctl(m_kvmfd, KVM_GET_SUPPORTED_CPUID, kvm_cpuid) < 0) throw std::runtime_error("KVM_GET_SUPPORTED_CPUID failed");
  /* Make sure the VM has the same CPUID as the host (this takes a bit of time, and can be optimized
   * significantly) */
  filter_cpuid(kvm_cpuid);

  // allocate the single virtual CPU core for the virtual machine.
  m_cpufd = ioctl(m_vmfd, KVM_CREATE_VCPU, 0);
  // init the cpuid
  int cpuid_err = ioctl(m_cpufd, KVM_SET_CPUID2, kvm_cpuid);
  if (cpuid_err != 0) {
    printf("%d %d\n", cpuid_err, errno);
    throw std::runtime_error("KVM_SET_CPUID2 failed");
  }

  // allocate the "run state"
  m_run = (struct kvm_run *)mmap(NULL, kvm_run_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_cpufd, 0);

  struct kvm_regs regs = {
      .rflags = 0x2,
  };
  ioctl(m_cpufd, KVM_SET_REGS, &regs);
  struct kvm_sregs sregs;
  ioctl(m_cpufd, KVM_GET_SREGS, &sregs);
  sregs.cs.base = 0;
  ioctl(m_cpufd, KVM_SET_SREGS, &sregs);
  /* Because KVM might have different initial register states on different machines,
   * we need to cache the state early on in the lifecycle for the "reset" procedure
   * that facilitates cleaning virtines efficiently
   */
  {
    auto regs = read_regs();
    regs.rip = regs.rsp = 0x8000;
    write_regs(regs);
  }

  // save the initial reset state
  save_reset_state();



  free(kvm_cpuid);
}


void wasp::Virtine::save_reset_state(void) {
  ioctl(m_cpufd, KVM_GET_REGS, &rst().regs);
  ioctl(m_cpufd, KVM_GET_SREGS, &rst().sregs);
  ioctl(m_cpufd, KVM_GET_FPU, &rst().fpu);
  rst().regions.clear();
}


void wasp::Virtine::save_reset_state(std::vector<ResetMemory> &&regions) {
  ioctl(m_cpufd, KVM_GET_REGS, &rst().regs);
  ioctl(m_cpufd, KVM_GET_SREGS, &rst().sregs);
  ioctl(m_cpufd, KVM_GET_FPU, &rst().fpu);
  rst().regions = std::move(regions);
}


bool wasp::Virtine::allocate_memory(size_t memsz) {
  if (memsz == 0) throw std::logic_error("zero memory size does not make sense");
  if ((memsz & 0xFFF) != 0) throw std::logic_error("virtine memory sizes must be page-aligned");

  // if the memory size is already correct, don't change anything
  // if (memsz == m_memsz) return true;

  void *new_addr = mmap(NULL, memsz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
  if (new_addr == MAP_FAILED) return false;

  struct kvm_userspace_memory_region code_region = {
      .slot = 0,             // allocate in slot zero
      .guest_phys_addr = 0,  // allocate at 0x0000'0000
      .memory_size = memsz,
      .userspace_addr = (uint64_t)new_addr,
  };

  // actually map the region into the virtine
  ioctl(m_vmfd, KVM_SET_USER_MEMORY_REGION, &code_region);

  // now, if we had an old region, free it
  if (m_mem != 0) munmap(m_mem, m_memsz);

  m_memsz = memsz;
  m_mem = new_addr;

  return true;
}

wasp::Virtine::~Virtine(void) {
  int kvm_run_size = ioctl(m_kvmfd, KVM_GET_VCPU_MMAP_SIZE, nullptr);
  munmap(m_run, kvm_run_size);
  close(m_cpufd);
  close(m_vmfd);
  if (m_mem != 0) munmap(m_mem, m_memsz);
}


bool wasp::Virtine::reset(void) {
  /* TODO: can this instead be done with the linux memory system? */
  // if (m_mem != NULL) memset(m_mem, 0, m_memsz);

  write_regs(rst().regs);
  write_sregs(rst().sregs);
  write_fpu(rst().fpu);

  for (auto &r : rst().regions) {
    // printf("reset region 0x%08lx-%08lx\n", r.address, r.address + r.size);
    void *addr = translate<void>(r.address);
    memcpy(addr, r.data, r.size);
  }

  return true;
}


wasp::ExitReason wasp::Virtine::run(void) {
  wasp::VirtineRegisters r;
top:
  /* Importantly, the first thing we do is call into ioctl to run. This helps lower latencies
   * significantly */
  int err = ioctl(m_cpufd, KVM_RUN, NULL);

  if (err < 0 && (errno != EINTR && errno != EAGAIN)) {
    fprintf(stderr, "KVM_RUN failed: '%s'\n", strerror(errno));
    return wasp::ExitReason::Crashed;  // dunno
  }

  int stat = m_run->exit_reason;
  // printf("stat = %d\n", stat);

  // handle hypercalls early
  if (stat == KVM_EXIT_IO) {
    // the port, 0xFA is a special exit port in wasp
    if (m_run->io.port == 0xFA) {
      return wasp::ExitReason::Exited;
    }
    // otherwise, it was a regular old hypercall that you need to interrogate
    return wasp::ExitReason::HyperCall;
  }

  switch (stat) {
    case KVM_EXIT_HLT:
      return wasp::ExitReason::Halted;
    case KVM_EXIT_INTR:
      return wasp::ExitReason::Interrupted;
    case KVM_EXIT_SHUTDOWN:
    case KVM_EXIT_INTERNAL_ERROR:
    case KVM_EXIT_FAIL_ENTRY:
    case KVM_EXIT_MMIO: {
#if 0
      wasp::VirtineRegisters r;
      read_regs(r);
      fprintf(stderr, "crash at 0x%016llx. stat=%d\n", r.rip, stat);
      fprintf(stderr, "rbx:0x%016llx\n", r.rbx);
      fprintf(stderr, "rsp:0x%016llx\n", r.rsp);
      uint8_t *d = translate<uint8_t>(r.rip);
      fprintf(stderr, "code:");
      for (int i = 0; i < 32; i++) {
        fprintf(stderr, " %02x", d[i]);
      }
      fprintf(stderr, "\n");
#endif
      // probably a triple fault
      return wasp::ExitReason::Crashed;
    }
    default:
      return wasp::ExitReason::Unknown;
  }

  return wasp::ExitReason::Unknown;
}

void wasp::Virtine::read_regs(wasp::VirtineRegisters &r) { ioctl(m_cpufd, KVM_GET_REGS, &r); }
void wasp::Virtine::write_regs(const wasp::VirtineRegisters &r) { ioctl(m_cpufd, KVM_SET_REGS, &r); }
void wasp::Virtine::read_sregs(wasp::VirtineSRegs &r) { ioctl(m_cpufd, KVM_GET_SREGS, &r); }
void wasp::Virtine::write_sregs(const wasp::VirtineSRegs &r) { ioctl(m_cpufd, KVM_SET_SREGS, &r); }
void wasp::Virtine::read_fpu(wasp::VirtineFPU &r) { ioctl(m_cpufd, KVM_GET_FPU, &r); }
void wasp::Virtine::write_fpu(const wasp::VirtineFPU &r) { ioctl(m_cpufd, KVM_SET_FPU, &r); }

bool wasp::Virtine::load_binary(const char *path, off_t rip) {
  FILE *stream = fopen(path, "r");
  if (stream == NULL) return false;

  fseek(stream, 0, SEEK_END);
  size_t sz = ftell(stream);
  fseek(stream, 0, SEEK_SET);
  void *mem = translate<void>(rip);
  // printf("mem: %p\n", mem);
  fread(mem, sz, 1, stream);
  fclose(stream);

  auto regs = read_regs();
  regs.rip = rip;
  write_regs(regs);

  return true;
}

bool wasp::Virtine::load_raw(void *bin, size_t size, off_t rip) {
  // TODO: make sure there is enough ram lol
  if (rip + size > m_memsz) return false;

  void *mem = translate<void>(rip);
  memcpy(mem, bin, size);

  auto regs = read_regs();
  regs.rip = rip;
  write_regs(regs);


  return true;
}

// utility functions
static inline void host_cpuid(struct cpuid_regs *regs) {
  __asm__ volatile("cpuid" : "=a"(regs->eax), "=b"(regs->ebx), "=c"(regs->ecx), "=d"(regs->edx) : "0"(regs->eax), "2"(regs->ecx));
}

static void filter_cpuid(struct kvm_cpuid2 *kvm_cpuid) {
  unsigned int i;
  struct cpuid_regs regs;

  /*
   * Filter CPUID functions that are not supported by the hypervisor.
   */
  for (i = 0; i < kvm_cpuid->nent; i++) {
    struct kvm_cpuid_entry2 *entry = &kvm_cpuid->entries[i];

    switch (entry->function) {
      case 0:

        regs = (struct cpuid_regs){
            .eax = 0x00,

        };
        host_cpuid(&regs);
        /* Vendor name */
        entry->ebx = regs.ebx;
        entry->ecx = regs.ecx;
        entry->edx = regs.edx;
        break;
      case 1:
        /* Set X86_FEATURE_HYPERVISOR */
        if (entry->index == 0) entry->ecx |= (1 << 31);
        /* Set CPUID_EXT_TSC_DEADLINE_TIMER*/
        if (entry->index == 0) entry->ecx |= (1 << 24);
        break;
      case 6:
        /* Clear X86_FEATURE_EPB */
        entry->ecx = entry->ecx & ~(1 << 3);
        break;
      case 10: { /* Architectural Performance Monitoring */
        union cpuid10_eax {
          struct {
            unsigned int version_id : 8;
            unsigned int num_counters : 8;
            unsigned int bit_width : 8;
            unsigned int mask_length : 8;
          } split;
          unsigned int full;
        } eax;

        /*
         * If the host has perf system running,
         * but no architectural events available
         * through kvm pmu -- disable perf support,
         * thus guest won't even try to access msr
         * registers.
         */
        if (entry->eax) {
          eax.full = entry->eax;
          if (eax.split.version_id != 2 || !eax.split.num_counters) entry->eax = 0;
        }
        break;
      }
      default:
        /* Keep the CPUID function as -is */
        break;
    };
  }
}
