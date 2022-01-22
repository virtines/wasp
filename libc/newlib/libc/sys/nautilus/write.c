
#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "syscall.h"
#include "warning.h"

_ssize_t
write(int file, const void * ptr, size_t len)
{
	return _write_r(_REENT, file, ptr, len);
}

_ssize_t
_write_r(struct _reent * r, int file, const void * ptr, size_t len)
{
	int ret;

        ret = sys_write(file, ptr, len);
	if (ret < 0) {
		r->_errno = -ret;
		ret = -1;
	}

	return ret;
}
