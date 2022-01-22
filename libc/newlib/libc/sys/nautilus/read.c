#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "syscall.h"
#include "warning.h"

_ssize_t
read (int file, void * ptr, size_t len)
{
	return _read_r(_REENT, file, ptr, len);
}

_ssize_t
_read_r(struct _reent * p, int file, void * ptr, size_t len)
{
	int ret;

	ret = sys_read(file, ptr, len);
	if (ret < 0) {
		p->_errno = -ret;
		ret = -1;
	}

	return ret;
}
