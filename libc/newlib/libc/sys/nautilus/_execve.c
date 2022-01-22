#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "syscall.h"
#include "warning.h"

int
_execve(const char  * name,
        char * const * argv,
        char * const * env)
{
	return _execve_r(_REENT, name, argv, env);
}

int
_execve_r(struct _reent * ptr,
        const char  * name,
        char * const * argv,
        char * const * env)
{
	int ret;

	ret = sys_execve(name, argv, env);
	if (ret < 0) {
		ptr->_errno = -ret;
		ret = -1;
	}

	return ret;
}
