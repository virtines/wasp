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

#include <stdint.h>

#define CAT_(a, b) a##b
#define CAT(a, b) CAT_(a, b)
#define VARNAME(Var) CAT(Var, __LINE__)
#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

// A virtine is just a function located in a certain segment
#define virtine __attribute__((section("$__VIRTINE__$"), noinline))

#define HCALL_exit 0
#define HCALL_close 1
#define HCALL_fstat 2
#define HCALL_link 3
#define HCALL_lseek 4
#define HCALL_open 5
#define HCALL_read 6
#define HCALL_sbrk 7
#define HCALL_times 8
#define HCALL_unlink 9
#define HCALL_write 10
#define HCALL_gettimeofday 11
#define HCALL_isatty 12

#define VIRTINE_ALLOW_EXIT (1 << 0)
#define VIRTINE_ALLOW_CLOSE (1 << 1)
#define VIRTINE_ALLOW_FSTAT (1 << 2)
#define VIRTINE_ALLOW_LINK (1 << 3)
#define VIRTINE_ALLOW_LSEEK (1 << 4)
#define VIRTINE_ALLOW_OPEN (1 << 5)
#define VIRTINE_ALLOW_READ (1 << 6)
#define VIRTINE_ALLOW_SBRK (1 << 7)
#define VIRTINE_ALLOW_TIMES (1 << 8)
#define VIRTINE_ALLOW_UNLINK (1 << 9)
#define VIRTINE_ALLOW_WRITE (1 << 10)
#define VIRTINE_ALLOW_GETTIMEOFDAY (1 << 11)
#define VIRTINE_ALLOW_ISATTY (1 << 12)

#define VIRTINE_ALLOW_ALL (~0LLU)
#define VIRTINE_BLOCK_ALL (0LLU)
struct virtine_config {
  uint64_t hypercall_whitelist;
};

#define virtine_cfg(config)                                                    \
  __attribute__((section("$__VIRTINE__cfg=" #config), noinline))

#define VIRTINE_LINE_CONFIG_NAME(name) "$__VIRTINE__cfg=" #name

#define virtine_whitelist(wl)                                                  \
  struct virtine_config VARNAME(__config_) = {                                 \
      .hypercall_whitelist = (wl),                                             \
  };                                                                           \
  __attribute__((section("$__VIRTINE__cfg=__config_" STRINGIFY(__LINE__)),     \
                 noinline))

#define virtine_strict virtine_whitelist(VIRTINE_BLOCK_ALL)

#define virtine_inline_config(cfg)                                             \
  struct virtine_config VARNAME(__config_) = {cfg};                            \
  __attribute__((section("$__VIRTINE__cfg=__config_" STRINGIFY(__LINE__)),     \
                 noinline))
