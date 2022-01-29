#include <stdio.h>
#include <wasp/Virtine.h>
#include <memory>
#include <bench.h>
#include <sys/mman.h>
#include <wasp/Cache.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <sys/ioctl.h>

#define NPOINTS 100
long data[NPOINTS];

// measures the latency to allocate a virtine context in cycles
int main(int argc, char **argv) {
  size_t memsz = 4096;



  int kvmfd = open("/dev/kvm", O_RDWR);

  for (int i = 0; i < NPOINTS; i++) {
    auto start = wasp::tsc();

    int vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0);
    // allocate the single virtual CPU core for the virtual machine.
    int cpufd = ioctl(vmfd, KVM_CREATE_VCPU, 0);
    auto end = wasp::tsc();

    data[i] = end - start;

		close(cpufd);
		close(vmfd);
    continue;


#if 0
    auto start = wasp::tsc();
    wasp::Virtine v;
		//v.set_unowned_memory(memsz, new_addr);
		v.allocate_memory(memsz);
    v.load_raw(MINIMAL_VIRTINE, MINIMAL_VIRTINE_SIZE, 0);
    // run until any exit

    v.run();
    auto end = wasp::tsc();
    data[i] = end - start;
#endif
  }


  printf("# trial, latency (cycles)\n");
  for (int i = 0; i < NPOINTS; i++) {
    printf("%d, %lu\n", i, data[i]);
  }
}
