
#include "rt/duktape.h"
#define DUK_USE_DATE_GET_LOCAL_TZOFFSET(d) 0
#define DUK_USE_DATE_GET_NOW(d) 0

#include "rt/duktape.c"

#include <getopt.h>
#include <chrono>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>  //std::stringstream

#include "rt/base64.h"


static duk_ret_t native_base64(duk_context *ctx) {
  duk_json_encode(ctx, 0);
  const char *val = duk_get_string(ctx, 0);
  char *b64 = encode(val);
  duk_push_lstring(ctx, b64, strlen(b64));
  free(b64);
  return 1;
}

using namespace std::chrono;
inline uint64_t time_us(void) {
  return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
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


class NativeJSEngine {
 public:
  void evaluate(const std::string &code) {
    duk_context *ctx = NULL;
    int i;
    duk_int_t rc;


    ctx = duk_create_heap_default();
    if (ctx == NULL) {
      printf("context is null!\n");
      return;
    }


    duk_push_c_function(ctx, native_print, DUK_VARARGS);
    duk_put_global_string(ctx, "print");
    eval_string(ctx, "function hcall_return(arg) { /* Do Nothing */ }");
    if (duk_peval_lstring(ctx, code.data(), code.size()) != 0) {
      duk_safe_to_stacktrace(ctx, -1);
      const char *res = duk_get_string(ctx, -1);
      printf("DUKTAPE ERROR: %s\n", res ? res : "null");
    }

    duk_pop(ctx);

    duk_destroy_heap(ctx);
  }
};


int main(int argc, char **argv) {
  int opt;
  while ((opt = getopt(argc, argv, "")) != -1) {
  }

  if (optind > argc) {
    fprintf(stderr, "usage: js [st] file.js\n");
    exit(EXIT_FAILURE);
  }

  NativeJSEngine engine;


  std::ifstream t(argv[optind]);
  std::stringstream buffer;
  buffer << t.rdbuf();
  std::string code = buffer.str();  // str holds the content of the file
  printf("# trial, latency\n");
  for (int i = 0; i < 1000; i++) {
    auto start = time_us();
		engine.evaluate(code);
    auto end = time_us();
    printf("%d,%lu\n", i, end - start);
  }
}
