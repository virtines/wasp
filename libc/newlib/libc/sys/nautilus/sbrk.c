#include "config.h"
#include <reent.h>
#include <_syslist.h>
#include <errno.h>
#include "syscall.h"
#include "warning.h"

void*
sbrk(ptrdiff_t incr)
{
	return _sbrk_r(_REENT, incr);
}

void*
_sbrk_r (struct _reent * ptr, ptrdiff_t incr)
{
	ssize_t ret = sys_sbrk(incr);
	if (ret <= 0) {
		ptr->_errno = ENOMEM;
		ret = 0;
	}

	return (void*) ret;
}
