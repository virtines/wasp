#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include "warning.h"

int
link (const char * existing, const char * new)
{
	return _link_r(_REENT, existing, new);
}

int
_link_r(struct _reent * ptr, const char * existing, const char * new)
{
	ptr->_errno = EMLINK;
	return -1;
}
