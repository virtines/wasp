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

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>

// this is the interface for the clang pass
void wasp_run_virtine(const char *code, size_t codesz, size_t memsz, void *arg,
                      size_t argsz, void *config);

#ifdef __cplusplus
}; // extern "C"
#endif
