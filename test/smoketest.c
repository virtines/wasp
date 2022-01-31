#include <virtine.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


virtine int smoketest_square(int val) {
	return val * val;
}

virtine int smoketest_square_ref(int *r) {
	*r *= *r;
	return 0;
}


int main(void) {
	printf("running smoke test. If this program does not panic, wasp works as expected.\n");
	int val = 3;
	smoketest_square_ref(&val);
	assert(val == 9);


	val = smoketest_square(3);
	printf("val=%d\n", val);
	assert(val == 9);
	printf("If you see this, it didn't panic!\n");
	return 0;
}
