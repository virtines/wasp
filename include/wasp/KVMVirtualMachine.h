#pragma once

#include <wasp/VirtualMachine.h>
#include <linux/kvm.h>

namespace wasp {
  class KVMVirtualMachine : public wasp::VirtualMachine {
   public:
    KVMVirtualMachine(void);
    virtual ~KVMVirtualMachine(void);

    // ^wasp::VirtualMachine
    virtual void add_mapping(void *host, off_t guest, size_t npages);
    virtual void remove_mapping(off_t addr);
    virtual uint64_t hcall_arg(int index) const;
    virtual void hcall_return(uint64_t value);
    // reset to the initial state
    virtual void reset(void);


   protected:
		void read_regs(void);
    // The kvm file descriptors for accessing the KVM hypervisor
    int m_kvmfd = -1;
    int m_vmfd = -1;
    int m_cpufd = -1;
    struct kvm_run *m_run = NULL;

    struct kvm_regs regs;
		struct kvm_sregs sregs;
		struct kvm_fpu fpu;
  };
}  // namespace wasp
