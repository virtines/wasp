
#include "rt/duktape.h"
#define DUK_USE_DATE_GET_LOCAL_TZOFFSET(d) 0
#define DUK_USE_DATE_GET_NOW(d) 0

#include "rt/duktape.c"

#include <chrono>


using namespace std::chrono;
inline uint64_t time_us(void) {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}


static duk_ret_t native_print(duk_context *ctx) {
  duk_push_string(ctx, " ");
  duk_insert(ctx, 0);
  duk_join(ctx, duk_get_top(ctx) - 1);
  printf("%s\n", duk_to_string(ctx, -1));
  return 0;
}




static void eval_string(duk_context *ctx, const char *expr) {
  int rc = duk_peval_string(ctx, expr);
  if (rc != 0) {
    duk_safe_to_stacktrace(ctx, -1);
    const char *res = duk_get_string(ctx, -1);
    printf("%s\n", res ? res : "null");
  } else {
  }
  duk_pop(ctx);
}


void run_js() {
  int i;
  duk_int_t rc;

  duk_context *ctx = duk_create_heap_default();

  auto start = time_us();

  duk_push_c_function(ctx, native_print, DUK_VARARGS);
  duk_put_global_string(ctx, "print");
  eval_string(ctx, "function handler(arg) { return arg.toUpperCase(); }");
  eval_string(ctx, "var _arg = 'hello world';");
  eval_string(ctx, "var res = handler(_arg);");
	
  auto end = time_us();
  printf("%d,%lu\n", i, end - start);
  duk_destroy_heap(ctx);
}

int main() {
  printf("# trial, latency\n");
  for (int i = 0; i < 1000; i++) {
    run_js();
  }
}
