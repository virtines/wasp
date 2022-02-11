#include <wasp/Virtine.h>
#include <stdlib.h>
#include "include/bench.h"

int main(int argc, char **argv) {
	wasp::Virtine v;


	v.allocate_memory(4096);

	v.load_raw(MINIMAL_VIRTINE, MINIMAL_VIRTINE_SIZE, 0);

	while (1) {
		auto res = v.run();
		printf("res=%d\n", res);
		if (res == wasp::ExitReason::Exited) {
			break;
		}
	}

}
