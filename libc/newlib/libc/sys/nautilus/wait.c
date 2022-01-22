#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "syscall.h"
#include "warning.h"

int
wait(int * status)
{
	return _wait_r(_REENT, status);
}

int
_wait_r(struct _reent * ptr, int * status)
{
	int ret;

	/* create a child process */
	ret = sys_wait(status);
	if (ret < 0) {
		ptr->_errno = -ret;
		ret = -1;
	}

	return ret;
}
