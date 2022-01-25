[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

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
  * [Code Structure](#code-structure)
  * [Acknowledgements](#acknowledgements)
  * [License](#license)
  * [Contact](#contact)

## Paper
* [Isolating Functions at the Hardware Limit with Virtines](#)<br>
Nicholas C. Wanninger, Joshua J. Bowden, Kyle C. Hale<br>
The 17th European Conference on Computer Systems (EuroSys '22, to appear)

## Build Instructions

```bash
make
sudo make install
```

Yeah its pretty easy.

## Using

Just include it as a shared library when building:

```bash
clang++ -lwasp main.cpp -o main
```

## Run Virtine Tests
TODO

## Reproduce Paper Results
TODO

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
