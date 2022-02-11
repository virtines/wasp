#include <wasp/KVMVirtualMachine.h>
#include <wasp/Virtine.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>



struct cpuid_regs {
  int eax;
  int ebx;
  int ecx;
  int edx;
};
#define MAX_KVM_CPUID_ENTRIES 100
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


static int g_kvmfd = -1;

wasp::KVMVirtualMachine::KVMVirtualMachine(void) {
  if (g_kvmfd == -1) {
    g_kvmfd = open("/dev/kvm", O_RDWR);
    if (g_kvmfd == -1) throw std::runtime_error("unable to open /dev/kvm");
  }

  m_kvmfd = g_kvmfd;


  // create a VM
  m_vmfd = ioctl(m_kvmfd, KVM_CREATE_VM, 0);
  // allocate the single virtual CPU core for the virtual machine.
  m_cpufd = ioctl(m_vmfd, KVM_CREATE_VCPU, 0);


  // get cpuid info
  struct kvm_cpuid2 *kvm_cpuid = NULL;
  kvm_cpuid = (kvm_cpuid2 *)calloc(1, sizeof(*kvm_cpuid) + MAX_KVM_CPUID_ENTRIES * sizeof(*kvm_cpuid->entries));
  kvm_cpuid->nent = MAX_KVM_CPUID_ENTRIES;
  if (ioctl(m_kvmfd, KVM_GET_SUPPORTED_CPUID, kvm_cpuid) < 0) throw std::runtime_error("KVM_GET_SUPPORTED_CPUID failed");
  /* Make sure the VM has the same CPUID as the host (this takes a bit of time, and can be optimized
   * significantly) */
  filter_cpuid(kvm_cpuid);


  // init the cpuid
  int cpuid_err = ioctl(m_cpufd, KVM_SET_CPUID2, kvm_cpuid);
  if (cpuid_err != 0) {
    printf("%d %d\n", cpuid_err, errno);
    throw std::runtime_error("KVM_SET_CPUID2 failed");
  }

  free(kvm_cpuid);

  int kvm_run_size = ioctl(m_kvmfd, KVM_GET_VCPU_MMAP_SIZE, nullptr);
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
}

wasp::KVMVirtualMachine::~KVMVirtualMachine(void) {}


// TODO:
void wasp::KVMVirtualMachine::add_mapping(void *host, off_t guest, size_t npages) {}
void wasp::KVMVirtualMachine::remove_mapping(off_t addr) {}
uint64_t wasp::KVMVirtualMachine::hcall_arg(int index) const { return 0; }
void wasp::KVMVirtualMachine::hcall_return(uint64_t value) {}
