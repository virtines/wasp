#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "syscall.h"
#include "warning.h"

_off_t
lseek (int file, _off_t ptr, int dir)
{
	return _lseek_r(_REENT, file, ptr, dir);
}

_off_t
_lseek_r(struct _reent * p, int file, _off_t ptr, int dir)
{
	int ret;	

	ret = sys_lseek(file, ptr, dir);
	if (ret < 0) {
		p->_errno = -ret;
		ret = -1;
	}

	return ret;
}
