#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "warning.h"

int
unlink (const char * name)
{
	return _unlink_r(_REENT, name);
}

int
_unlink_r(struct _reent * ptr, const char * name)
{
	ptr->_errno = ENOENT;
	return -1;
}
