
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <reent.h>
#include <malloc.h>
#include <_syslist.h>
#include "syscall.h"

/*
 * Notes:
 *	- man 2 sigreturn
 *	- man 7 signal
*/

static void
newlib_signal_dispatcher(int signum)
{
	// assume to be called in user context
	struct _reent* reent = _REENT;

	if (signum < 0 || signum >= NSIG) {
		reent->_errno = EINVAL;
		return;
	}

	if(reent->_sig_func == NULL) {
		reent->_errno = ENOENT;
		return;
	}

	_sig_func_ptr func = reent->_sig_func[signum];

	if(func == SIG_DFL) {
		fprintf(stderr, "Caught unhandled signal %d, terminating\n", signum);
		// terminate if no signal handler registered
		sys_exit(128 + signum);
	} else if(func == SIG_IGN) {
		// ignore
	} else if(func == SIG_ERR) {
		reent->_errno = EINVAL;
	} else {
		// finally call user code
		func(signum);
	}
}

int
pthread_sigmask (int how, const sigset_t * set, sigset_t * oset)
{
	return ENOSYS;
}

int
signalstack(const stack_t * ss, stack_t * oss)
{
	return ENOSYS;
}

int
_init_signal_r (struct _reent * ptr)
{
	int i;

	if (ptr->_sig_func == NULL) {
		ptr->_sig_func = (_sig_func_ptr *)_malloc_r (ptr, sizeof (_sig_func_ptr) * NSIG);

		if (ptr->_sig_func == NULL) {
			return -1;
		}

		for (i = 0; i < NSIG; i++) {
			ptr->_sig_func[i] = SIG_DFL;
		}

		sys_signal(newlib_signal_dispatcher);
	}

	return 0;
}

_sig_func_ptr
_signal_r (struct _reent * ptr, int sig, _sig_func_ptr func)
{
	_sig_func_ptr old_func;

	if (sig < 0 || sig >= NSIG) {
		ptr->_errno = EINVAL;
		return SIG_ERR;
	}

	if (ptr->_sig_func == NULL && _init_signal_r (ptr) != 0) {
		return SIG_ERR;
	}

	old_func = ptr->_sig_func[sig];
	ptr->_sig_func[sig] = func;

	return old_func;
}

int
_sigaction_r(struct _reent * ptr, int sig, const struct sigaction * act, struct sigaction * oact)
{
	return 0;
}

#ifndef _REENT_ONLY

int
sigaction(int sig, const struct sigaction * act, struct sigaction * oact)
{
	return _sigaction_r (_REENT, sig, act, oact);
}

_sig_func_ptr
signal(int sig, _sig_func_ptr func)
{
	return _signal_r (_REENT, sig, func);
}

int 
_init_signal()
{
	return _init_signal_r (_REENT);
}

#endif
