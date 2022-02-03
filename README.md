[![Build Status](https://app.travis-ci.com/virtines/wasp.svg?branch=main)](https://app.travis-ci.com/virtines/wasp)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# Wasp

Wasp is an embeddable micro-hypervisor designed to support _virtines_, lightweight
isolated execution contexts for individual functions. Virtines can either be created
manually by invoking a runtime API, or automatically by using our Clang/LLVM compiler
extensions. In the latter case, a developer can create a virtine by simply adding
a keyword to a C function:

```C
virtine int foo(int arg) {
	// code
} 
```

- [Wasp](#wasp)
  * [Paper](#paper)
  * [Build Instructions](#build-instructions)
  * [Running Basic Virtine Tests](#running-virtine-tests)
  * [Reproducing Paper Results](#reproducing-paper-results)
  * [Embedding Wasp](#embedding-wasp)
  * [Code Structure](#code-structure)
  * [Acknowledgements](#acknowledgements)
  * [License](#license)
  * [Contact](#contact)

## Paper
* [Isolating Functions at the Hardware Limit with Virtines](https://nickw.io/papers/eurosys22.pdf)<br>
Nicholas C. Wanninger, Joshua J. Bowden, Kyle C. Hale<br>
The 17th European Conference on Computer Systems (EuroSys '22, to appear)

### Experimental Platform in the Paper
Note that the experiments in the paper were run on a Dell PowerEdge R6415
running Linux, specifically Fedora Server 34 with stock kernel version 5.9.12.
This machine has an AMD EPYC 7281 (Naples; 16 cores; 2.69 GHz) CPU with 32 GB
DDR4. We disabled hyperthreading, turbo boost, and DVFS to mitigate measurement
noise. We used gcc 10.2.1 to compile Wasp (C/C++), clang 10.0.1 for our C-based
virtine language extensions, and NASM v2.14 for assembly-only virtines. The data and plots
used in the paper can be found in `data_example/


## Build Instructions

### Environment Setup

Wasp can be built and evaluated on any Linux x86 hardware that supports
hardware virtualization through KVM. For evaluating the artifacts, we recommend
an allocation on Cloudlab or Chameleon Cloud, as outlined below:

**Cloudlab Configuration**:

- Apply for a Cloudlab account if you do not have one
- Once logged in, Click `Experiments` then `Create Experiment Profile`
- Upload the `cloudlab.profile` file provided in the root of this repo
- Create the instance using that profile and SSH into it following cloudlab's docs

**Chameleon Configuration**:
- Apply for a Chameleon account if you do not already have one.
- Allocate a lease for one host (any x86 host with virtualization hardware should do, though we've tested on the default Haswell and Skylake nodes). 
- Select "Images" from the side bar and search for the "CC-Ubuntu20.04". Create an instance from this image by selecting
"Launch" on the right. Tie this instance to the lease you created before under the "Reservation" drop-down. Make sure
you set up a key pair and a floating IP so you can ssh to the instance. 
- Once the instance launches, ssh to it using the default Chameleon account (e.g., `ssh cc@192.168.1.1`). 

Now you can proceed to building and running Wasp, described below. 

### Prerequisites 

The following packages are necessary to get Wasp running:
- `nasm`
- `libcurl` dev headers (`libcurl-dev` on recent Ubuntu is an aliased package; we had to use `libcurl4-openssl-dev`)
- `clang` version 10 or newer
- `llvm` and `llvm-dev`
- `cmake` 
- `python3.8-venv` (for plotting)
- `openssl` (only for OpenSSL example)

Additionally, you must be on a Linux box with KVM support (you can check with
`lsmod | grep kvm`), i.e., a baremetal machine or one that supports nested
virtualization.  Wasp only support x86_64 (Intel, AMD) chips that have hardware
virtualization extensions at the moment. 

For example, on an Ubuntu 20.04 LTS machine:

```bash
sudo apt update
sudo apt install -y cmake nasm llvm llvm-dev clang libcurl4-openssl-dev python3.8-venv openssl
```

### Building and Installing

```bash
git clone https://github.com/virtines/wasp.git
cd wasp
make
sudo make install
```
The build should take less than a minute or so. 
`make install` simply copies the resulting binaries to `/usr/local/{lib,bin,include}`, and will
not run any programs other than `mkdir` and `install`.

## Running Virtine Tests

To ensure Wasp is functional, we have included a "smoke test" in the repo. You can run it like so (it should complete pretty
much immediately):

```bash
make smoketest
```

If the smoketest doesn't panic, Wasp is functional at this point. We tested
this most recently on Chameleon Cloud Ubuntu 20.04.3 LTS (baremetal Skylake, 48
cores, Xeon Gold 6126) with Linux kernel version 5.4.0-91-generic.

If the smoke test does panic, it may be because `/dev/kvm` is owned by root.
It's advised to allow anyone to open `/dev/kvm` when gathering the artifacts.
To do so, you can run `sudo chmod 666 /dev/kvm`. Feel free to restore it after running
the artifact.

## Reproducing Paper Results

To re-run the experiments and reproduce relevant figures from the paper,
simply run:

```bash
make artifacts.tar
```

This will produce a `.tar` archive containing all relevant figures and data in `artifacts.tar`. It should take about
5 minutes to reproduce all results and plots this way. Here is
what is included from the paper:

- Context creation experiment (`fig8.pdf`); Figures 2 and 8 from the paper. Figure 8 is a superset of Figure 2. 
- Boot time breakdown (`data/table1.csv`); Table 1 from the paper.
- Mode latency experiment (`fig3.pdf`); Figure 3 from the paper.
- Echo server boot latency (`fig4.pdf`); Figure 4 from the paper.
- Virtine creation latency microbenchmark (`fig11.pdf`); Figure 11 from the paper.
- Effect of image size on virtine start-up latency  (`fig12.pdf`); Figure 12 from the paper.
- Virtine HTTP server performance (`fig13_tput.pdf`, `fig13_lat.pdf`); Figure 13 from the paper.
- OpenSSL data (`data/openssl.txt` and `data/openssl_baseline.txt`); Section 6.4 from the paper.
- JavaScript virtines performance (`fig14.pdf`); Figure 14 from the paper.


If you would like to run a single experiment, simply run `make data_figX` where X is the
id of the figure (fig14.pdf uses `data_fig14`, `fig13_lat.pdf` uses `data_fig13_lat`).

Due to the nature of microarchitectural differences across CPUs, the
data produced from some of these tests will vary slightly from machine to machine. In particular, we have noticed that
Figures 3 and 8 and Table 1 show some discrepancies between AMD and Intel; we also see some variance across
microarchitectural generations.  Once you've collected results, you can compare them to some of the reference
data we've collected in the `data_example` directory. See [here](#code-structure) for more.


## Embedding Wasp
Wasp can be used two ways: as a library (which the programmer invokes through API calls) or as a compiler extension. Directly interfacing
with Wasp can provide higher control over the execution of a virtine leading to lower
overheads. Wasp's compiler extensions ease development
significantly, but result in higher overheads and larger binaries due to their general-purpose functionality.

### Direct API Access

Wasp exposes a simple API to construct a virtine in `<wasp/Virtine.h>`. You'll notice
that the interface does not assume anything about the operation of the particular virtine,
including how it is built. When interfacing with the wasp API directly, virtine compilation
and invocation is up to the developer. An example of some minimal virtine code, as well as the code to invoke it
looks like this:

```cpp
// host.cpp
#include <wasp/Virtine.h>

int main(int argc, char **argv) {
	wasp::Virtine virtine;

	// allocate ram as a contiguous chunk of 16kb (in page alignments)
	virtine.allocate_memory(4096 * 4);

	// load a flat binary into the virtine at address 0x8000
	virtine.load_binary("virtine.bin", 0x8000);
	while (1) {
		// run the virtine until it exits by some mechanism
		//   (there will eventually be a timeout :^) )
		auto res = virtine.run();
		if (res == wasp::ExitReason::Hypercall) {
			// handle the hypercall by interfacing with the registers
		}
		if (res == wasp::ExitReason::Exited) break;
	}
	return 0;
}
```

Here is a very simple virtine written in assembly:

```asm
;; virtine.asm
[org 0x8000]
global _start
_start:
	;; magic exit hypercall
	out 0xFA, eax
```

In this case, the developer must write and compile the virtine code on their own. In this example, the virtine's image (`virtine.bin`) can
be assembled from the above code using `nasm` which will produce a 3 byte binary.

The final application can be compiled and run as follows:

```bash
nasm -fbin virtine.asm -o virtine.bin
gcc -lwasp host.cpp -o host
./host
```
This should produce no output. The virtine will simply exit immediately. See the `test/` directory for some more concrete examples.
(`test/js/host.cpp` showcases most of the available API functionality).

To improve performance, Wasp
provides caching mechanisms via `wasp::Cache`. If the above `host.cpp` is rewritten with caching,
only a few things change:

```cpp
// host.cpp
#include <wasp/Virtine.h>
#include <wasp/Cache.h>

int main(int argc, char **argv) {

	// allocate a cache of virtines that all have 16kb of memory
	wasp::Cache cache(4096 * 4);

	// load the binary on the cache, not each virtine
	cache.load_binary("virtine.bin", 0x8000);

	// pre-provision 12 virtines (arbitrarially)
	cache.ensure(12);

	// get a fresh virtine from the cache
	wasp::Virtine *v = cache.get();
	while (1) {
		// run the virtine, 
		auto res = v->run();
		if (res == wasp::ExitReason::Hypercall) {
			// handle the hypercall by interfacing with the registers
		}
		if (res == wasp::ExitReason::Exited) break;
	}
	// return the virtine once we are done
	cache.put(v);
	return 0;
}
```


### Virtine Compiler Extension (`vcc`)

The easiest way to use virtines is through `vcc`, which once built and installed
allows existing *C* programs to run in context using only a simple keyword. A trivial example looks like this:

```c
// main.c
#include <virtine.h>
#include <stdio.h>

virtine int square(int x) {
	return x * x;
}

int main(int argc, char **argv) {
	printf("%d\n", square(3));
	return 0;
}
```

If compiled with `vcc` (a drop-in for clang/gcc) a virtine for `square()` will be spawned and executed
whenever `square` is invoked. By default, the virtine utilizes *snapshotting* to decrease start-up latencies. This
can be disabled by defining the environment variable `WASP_NO_SNAPSHOT=1` if you wish to see its effect.
The compiler extension is currently in its early stages, and suffers from much of the weaknesses that
a typical C compiler might have (no full program analysis, pointer aliasing, inability to
inline functions from other C files, unsized pointers, etc.), but we imagine a future where a high-level
language like SML may solve many of those problems.


## Code Structure
All of the wasp runtime is implemented in `src/` and `include/`, the LLVM pass
is implemented in `pass/`, and `test/` contains a set of tests which produce
the data from the paper. The `data_example` directory contains example data
gathered from a few machines. For example `AMD-EPYC-7302P-cloudlab-c6525-25g/`
contains the data gathered from the machine using the `cloudlab.profile`, and
`Intel-XEON-Gold-6126-chameleon` is from an allocation on chameleon. See
`data_example/README.md` for more details on each machine's hadware and
software. 

## Acknowledgements

<img align="left" src="https://www.nsf.gov/images/logos/NSF_4-Color_bitmap_Logo.png" height=100/>

Wasp and the virtines codebase were made possible with support from the
United States [National Science
Foundation](https://nsf.gov) (NSF) via grants [CNS-1730689](https://nsf.gov/awardsearch/showAward?AWD_ID=1730689&HistoricalAwards=false), [REU-1757964](https://www.nsf.gov/awardsearch/showAward?AWD_ID=1757964), [CNS-1718252](https://www.nsf.gov/awardsearch/showAward?AWD_ID=1718252&HistoricalAwards=false), and [CNS-1763612](https://www.nsf.gov/awardsearch/showAward?AWD_ID=1763612&HistoricalAwards=false).<br>

## License
[![MIT License](http://seawisphunter.com/minibuffer/api/MIT-License-transparent.png)](https://github.com/HExSA-Lab/nautilus/blob/master/LICENSE.txt)

## Contact
The Wasp/virtines codebase is currently maintained by Nicholas Wanninger (ncw [at] u [dot] northwestern [dot] edu).
