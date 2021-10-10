
#pragma GCC push_options
#pragma GCC optimize ("O0")

int __attribute__ ((noinline)) fib(int n) {
    if (n < 2) return n;
		// this is a hacky way to force gcc to not constant fold
		asm ("");
    return fib(n-1) + fib(n-2);
}


#pragma GCC pop_options
