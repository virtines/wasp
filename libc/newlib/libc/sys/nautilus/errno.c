#include "config.h"
#include <reent.h>
#include <_ansi.h>
#include <_syslist.h>
#include "warning.h"

int * __errno(void)
{
	return &_REENT->_errno;
}
