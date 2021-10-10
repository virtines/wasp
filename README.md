# Wasp

Wasp is a library hypervisor designed to aid in the development
and deployment of virtines

## Build + Install

```bash
$ make
$ sudo make install
```

Yeah its pretty easy.

## Using

Just include it as a shared library when building:

```bash
$ clang++ -lwasp main.cpp -o main
```
