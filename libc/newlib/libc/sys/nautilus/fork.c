#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "syscall.h"
#include "warning.h"

int
_fork_r (struct _reent *ptr)
{
	int ret;

	/* create a child process */
	ret = sys_fork();
	if (ret < 0) {
		ptr->_errno = -ret;
		ret = -1;
	}

	return ret;
}
