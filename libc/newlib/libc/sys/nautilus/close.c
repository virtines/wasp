#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "syscall.h"
#include "warning.h"

int
close (int fildes)
{
	return _close_r(_REENT, fildes);
}

int
_close_r(struct _reent *ptr,
    int fildes)
{
	int ret;

    ret = sys_close(fildes);
    if (ret < 0) {
        ptr->_errno = -ret;
        ret = -1;
    }

    return ret;
}
