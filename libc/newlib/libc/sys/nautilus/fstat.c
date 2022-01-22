#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "warning.h"

int
fstat (int fildes, struct stat * st)
{
	return _fstat_r(_REENT, fildes, st);
}

int
_fstat_r (struct _reent * ptr, int fildes, struct stat * st)
{
	st->st_mode = S_IFCHR;
	return 0;
}
