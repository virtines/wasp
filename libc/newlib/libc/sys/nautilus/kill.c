#include "config.h"
#include <stdlib.h>
#include <signal.h>
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "warning.h"
#include "syscall.h"

int
kill (int pid, int sig)
{
	return _kill_r(_REENT, pid, sig);
}

int
_kill_r (struct _reent * ptr, int pid, int sig)
{
	if (sig < 0 || sig >= NSIG)
	{
		ptr->_errno = EINVAL;
		return -1;
	}

	int ret = sys_kill(pid, sig);
	if(ret) {
		ptr->_errno = ret;
	}
	return ret;
}

