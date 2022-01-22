#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <sys/times.h>
#include <errno.h>

clock_t
times(struct tms * buf)
{
	return _times_r(_REENT, buf);
}

clock_t
_times_r(struct _reent * ptr, struct tms *buf)
{
	ptr->_errno = EACCES;
	return -1;
}
