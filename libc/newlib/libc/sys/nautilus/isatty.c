#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "warning.h"

int
isatty (int file)
{
	return _isatty_r(_REENT, file);
}

int
_isatty_r(struct _reent * ptr, int file)
{
	return (file < 3);
}
