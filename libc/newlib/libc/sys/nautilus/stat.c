#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "syscall.h"
#include "warning.h"

int
stat (const char * file, struct stat * st)
{
	return _stat_r(_REENT, file, st);
}

int
_stat_r (struct _reent * ptr, const char * file, struct stat * st)
{
	int ret;

	if (!file && ! st) {
		ptr->_errno = EINVAL;
		return -1;
	}

	ret = sys_stat(file, st);
	if (ret < 0) {
		ptr->_errno = -ret;
		return -1;
	}

	return 0;
}
