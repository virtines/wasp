[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://app.travis-ci.com/virtines/wasp.svg?branch=main)](https://app.travis-ci.com/virtines/wasp)

# Wasp

Wasp is an embeddable micro-hypervisor designed to support _virtines_, lightweight,
isolated execution contexts for individual functions. Virtines can either be created
manually by invoking a runtime API, or automatically by using our Clang/LLVM compiler
extensions. In the latter case, a develper can create a virtine by simply adding
a keyword to a C function:

```C
virtine int foo (int arg) {
    // code
} 
```

- [Wasp](#wasp)
  * [Paper](#paper)
  * [Build Instructions](#build-instructions)
  * [Run Basic Virtine Tests](#run-virtine-tests)
  * [Reproduce Paper Results](#reproduce-paper-results)
  * [Embedding Wasp](#embedding-wasp)
  * [Code Structure](#code-structure)
  * [Acknowledgements](#acknowledgements)
  * [License](#license)
  * [Contact](#contact)

## Paper
* [Isolating Functions at the Hardware Limit with Virtines](https://nickw.io/papers/eurosys22.pdf)<br>
Nicholas C. Wanninger, Joshua J. Bowden, Kyle C. Hale<br>
The 17th European Conference on Computer Systems (EuroSys '22, to appear)

## Build Instructions

### Prerequisites and Building
- `nasm`
- `libcurl` dev headers (`libcurl-dev` on recent Ubuntu is an aliased package; we had to use `libcurl4-openssl-dev`)
- `clang` version 10 or newer
- `llvm` and `llvm-dev`
- `cmake` 

Additionally, you must be on a Linux box with KVM support (you can check with `lsmod | grep kvm`), i.e., a baremetal machine or one that supports nested virtualization.
Wasp only support x86 (Intel, AMD) at the moment. 

For example, on an Ubuntu machine:

```bash
sudo apt install -y cmake nasm llvm llvm-dev clang libcurl4-openssl-dev
```


### Building and Installing

```bash
git clone https://github.com/virtines/wasp.git
cd wasp
make
sudo make install
```

TODO: explain what make install is actually doing

## Run Virtine Tests

```bash
make smoketest
```

If the smoketest doesn't panic, Wasp is functional at this point. We tested
this most recently on Chameleon Cloud Ubuntu 20.04.3 LTS (baremetal Skylake, 48
cores, Xeon Gold 6126) Kernel version 5.4.0-91-generic.

## Reproduce Paper Results

To re-run the experiments and reproduce relevant figures from the paper,
simply run:

```bash
make artifacts
```

This will produce a `.tar` archive containing relevant figures and data in `artifacts.tar`. Here is
what is included from the paper:

- Context creation experiment (`fig8.pdf`); Figures 2 and 8 from the paper. Figure 8 is a superset of Figure 2. 
- Boot time breakdown (`data/table1.csv`); Table 1 form the paper.
- Mode latency experiment (`fig3.pdf`); Figure 3 from the paper.
- Echo server boot latency (`fig4.pdf`); Figure 4 from the paper.
- Virtine creation latency microbenchmark (`fig11.pdf`); Figure 11 from the paper.
- Effect of image size on virtine start-up latency  (`fig12.pdf`); Figure 12 from the paper.
- Virtine HTTP server performance (`fig13_tput.pdf`, `fig13_lat.pdf`); Figure 13 from the paper.
- TODO OpenSSL speed benchmark
- JavaScript virtines performance (`fig14.pdf`); Figure 14 from the paper.


TODO: explain how to run individual experiments. Discuss what kind of data they might see
on different machines. 

## Embedding Wasp

TODO: more detail

Just include it as a shared library when building:

```bash
clang++ -lwasp main.cpp -o main
```


## Code Structure
TODO

## Acknowledgements

<img align="left" src="https://www.nsf.gov/images/logos/NSF_4-Color_bitmap_Logo.png" height=100/>

Wasp and the virtines codebase were made possible with support from the
United States [National Science
Foundation](https://nsf.gov) (NSF) via grants [CRI-1730689](https://nsf.gov/awardsearch/showAward?AWD_ID=1730689&HistoricalAwards=false), [REU-1757964](https://www.nsf.gov/awardsearch/showAward?AWD_ID=1757964), [CNS-1718252](https://www.nsf.gov/awardsearch/showAward?AWD_ID=1718252&HistoricalAwards=false), and [CNS-1763612](https://www.nsf.gov/awardsearch/showAward?AWD_ID=1763612&HistoricalAwards=false).<br>

## License
[![MIT License](http://seawisphunter.com/minibuffer/api/MIT-License-transparent.png)](https://github.com/HExSA-Lab/nautilus/blob/master/LICENSE.txt)

## Contact
The Wasp/virtines codebase is currently maintained by Nicholas Wanninger (ncw [at] u [dot] northwestern [dot] edu).
