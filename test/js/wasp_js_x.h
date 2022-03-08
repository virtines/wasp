#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>

char* js_x(char* code1);
void get_cache_stats(int *misses, int *hits);
void foo();

#ifdef __cplusplus
}; // extern "C"
#endif