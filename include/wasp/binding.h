#pragma once

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
