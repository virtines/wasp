#include <fcntl.h>
 
extern void exit(int code);
extern int main ();


extern long __hypercall(int nr, long long a, long long b, long long c);
extern off_t __heap_top();
 
void _start() {
	int ex = main();
	exit(ex);
}
