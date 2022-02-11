#pragma once

#include <stdlib.h>
#include <stdint.h>

namespace wasp {

	// The base class for a virtual machine, which is used as a backend
	// for virtine execution, and is orchestrated by wasp::Virtines and their
	// caches
  class VirtualMachine {
   public:
    virtual void add_mapping(void *host, off_t guest, size_t npages) = 0;
    virtual void remove_mapping(off_t addr) = 0;

    // Access a hypercall argument
    virtual uint64_t hcall_arg(int index) const = 0;

    // set the hypercall return value (rax on x64)
    virtual void hcall_return(uint64_t value) = 0;
  };
}  // namespace wasp
