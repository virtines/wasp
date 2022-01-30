#include "duktape.h"
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include "virtine.h"

#include "base64.h"

extern long __hypercall(int nr, long long a, long long b, long long c);
extern long __save_reset_state(off_t start, off_t end);
extern off_t __heap_top();

int gettimeofday(struct timeval *__restrict __tv, void *__restrict __tz) { return -1; }


static duk_ret_t native_print(duk_context *ctx) {
  duk_push_string(ctx, " ");
  duk_insert(ctx, 0);
  duk_join(ctx, duk_get_top(ctx) - 1);
  printf("%s\n", duk_to_string(ctx, -1));
  return 0;
}


static const char *return_value = NULL;

static duk_ret_t native_hcall_return(duk_context *ctx) {
  duk_json_encode(ctx, 0);
  const char *ret = duk_get_string(ctx, 0);

  return_value = strdup(ret);
  return 0;
}

static void eval_string(duk_context *ctx, const char *expr) {
  int rc = duk_peval_string(ctx, expr);
  if (rc != 0) {
    duk_safe_to_stacktrace(ctx, -1);
    const char *res = duk_get_string(ctx, -1);
    printf("DUKTAPE ERROR: %s\n", res ? res : "null");
  }
  duk_pop(ctx);
}


extern int errno;
void virtine_main(void) {
  duk_context *ctx = NULL;
  int i;
  duk_int_t rc;


  ctx = duk_create_heap_default();
  if (ctx == NULL) {
    printf("context is null!\n");
    return;
  }
  int *do_teardown_ptr = NULL;
  int do_teardown = 1;  // *do_teardown_ptr;

  duk_push_c_function(ctx, native_print, DUK_VARARGS);
  duk_put_global_string(ctx, "print");

  duk_push_c_function(ctx, native_hcall_return, 1 /* nargs */);
  duk_put_global_string(ctx, "hcall_return");

  // save at this point
  __hypercall(0xFF, 0, __heap_top(), 0);

  size_t argsz = __hypercall(HCALL_GET_ARG, 0 /* NULL for buf gets size */, 0, 0);
  char *buf = (char *)malloc(argsz + 1);
  // actually get the argument now
  __hypercall(HCALL_GET_ARG, (unsigned long)(buf), argsz, 0);



  if (duk_peval_lstring(ctx, buf, argsz) != 0) {
    duk_safe_to_stacktrace(ctx, -1);
    const char *res = duk_get_string(ctx, -1);
    printf("DUKTAPE ERROR: %s\n", res ? res : "null");
  }

  duk_pop(ctx);


  if (do_teardown) duk_destroy_heap(ctx);


  // this will exit
  __hypercall(HCALL_RETURN, (unsigned long)return_value, strlen(return_value), 0);
}
