#pragma once
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

#include <linux/kvm.h>
#include <stdexcept>
#include <stdint.h>
#include <unistd.h>
#include <vector>

namespace wasp {

  enum ExitReason {
    Unknown = 0,
    Crashed = 1,
    Interrupted = 2,
    Halted = 3,  // generic case, typically not bad to just continue afterwards
    HyperCall = 4,
    Exited = 5,
  };

  using VirtineRegisters = kvm_regs;
  using VirtineSRegs = kvm_sregs;
  using VirtineFPU = kvm_fpu;

  struct ResetMemory {
    void *data;     // data to load (malloced)
    off_t address;  // where in the virtine to load this
    size_t size;    // the size of the data
  };

  // Stores the registers and regions of memory (if any) that the virtine
  // can use to reset.
  struct ResetState {
    VirtineRegisters regs;
    VirtineSRegs sregs;
    VirtineFPU fpu;
    std::vector<ResetMemory> regions;
  };

  // A Virtine represnts a virtine that can be spawned from a KVM filedescriptor.
  // This class maintains the information and state required to host a virtual
  // machine, it's CPU cores, and various memory states
  class Virtine final {
   public:
    // Use Virtine::create() instead
    Virtine();
    ~Virtine();

    /* This function allocates the virtine's memory (single, contiguous block) */
    bool allocate_memory(size_t memsz);

		// sets the memory to be some pointer given by the user. This does not free!
    bool set_unowned_memory(size_t memsz, void *mem);

    // save the reset state with no memory regions (clears existing ones)
    void save_reset_state(void);
    // save the reset state
    void save_reset_state(std::vector<ResetMemory> &&regions);

    template <typename T>
    T *translate(off_t addr) {
      if (addr > m_memsz) return NULL;
      return (T *)((off_t)m_mem + addr);
    }

    inline bool validate_length(off_t addr, size_t len) const {
      if (addr + len > m_memsz) return false;
      return true;
    }

    // returns a KVM_EXIT_* when the Virtine can't handle it.
    wasp::ExitReason run(void);

    void read_regs(VirtineRegisters &);
    auto inline read_regs(void) {
      wasp::VirtineRegisters regs;
      read_regs(regs);
      return regs;
    }
    void write_regs(const VirtineRegisters &);
    void read_sregs(VirtineSRegs &r);
    void write_sregs(const VirtineSRegs &);
    void read_fpu(VirtineFPU &dst);
    void write_fpu(const VirtineFPU &src);

    // Load a binary to a certain location and set the initial instruction pointer
    bool load_binary(const char *path, off_t rip);
    bool load_raw(void *bin, size_t size, off_t rip);
    // reset the virtine to it's original state (the `clean` operation)
    bool reset(void);

    // get the reset state
    inline ResetState &rst() {
      if (m_reset_mask != NULL) return *m_reset_mask;
      return m_reset;
    }

    inline void set_reset_mask(ResetState *mask) { m_reset_mask = mask; }

   private:
    bool m_halted = true;

    // fields
    void *m_mem = nullptr;
    size_t m_memsz = 0;

    // The kvm file descriptors for accessing the KVM hypervisor
    int m_kvmfd = -1;
    int m_vmfd = -1;
    int m_cpufd = -1;
    struct kvm_run *m_run = NULL;

    ResetState m_reset;
    // if this is not null, we use this instead
    ResetState *m_reset_mask = NULL;
  };

  static inline auto tsc(void) {
    uint32_t lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return lo | ((uint64_t)(hi) << 32);
  }
}  // namespace wasp
