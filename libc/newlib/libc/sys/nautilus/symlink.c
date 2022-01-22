#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "warning.h"


int
_symlink_r(struct _reent * ptr, const char * path1, const char * path2)
{
	ptr->_errno = ENOSYS;
	return -1;
}
