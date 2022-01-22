#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include "warning.h"


int
gettimeofday(struct timeval * ptimeval, void * ptimezone)
{
	return _gettimeofday_r(_REENT, ptimeval, ptimezone);
}

int
_gettimeofday_r(struct _reent *ptr, struct timeval * ptimeval, void *ptimezone)
{
	ptr->_errno = ENOSYS;
	return -1;
}
