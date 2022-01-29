#include <virtine.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

virtine_whitelist(VIRTINE_ALLOW_ALL) int foo(int a) {
	puts("in foo\n");
	return a * a;
}


int main() {
	printf("%d\n", foo(3));
	return 0;
}
