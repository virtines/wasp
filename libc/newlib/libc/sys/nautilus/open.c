#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "syscall.h"
#include "warning.h"

int
open (const char * file, int flags, int mode)
{
	return _open_r(_REENT, file, flags, mode);
}

int
_open_r(struct _reent * ptr, const char * file, int flags, int mode)
{
	int ret;

        ret = sys_open(file, flags, mode);
	if (ret < 0) {
		ptr->_errno = -ret;
		ret = -1;
	}

        return ret;
}
