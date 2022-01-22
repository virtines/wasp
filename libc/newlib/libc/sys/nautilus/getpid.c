#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "syscall.h"
#include "warning.h"

int
getpid()
{
	return _getpid_r(_REENT);
}

int
_getpid_r(struct _reent * ptr)
{
	int ret;

        ret = sys_getpid();
	if (ret < 0) {
		ptr->_errno = -ret;
		ret = -1;
	}

        return ret;
}
