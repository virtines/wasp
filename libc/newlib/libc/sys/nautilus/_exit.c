#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "syscall.h"

void _exit (int rc)
{
	sys_exit(rc);

	/* Convince GCC that this function never returns.  */
	for (;;)
		;
}
