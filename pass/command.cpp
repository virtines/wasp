#include <stdlib.h>
#include <string.h>
#include "command.h"
#include <sys/wait.h>
#include <errno.h>

extern char **environ;

ck::command::command(std::string exe) : m_exe(exe) {}


ck::command::~command(void) {
  if (pid != -1) {
		// XXX: What should we do here?
  }
}


void ck::command::arg(std::string arg) { m_args.push_back(arg); }


int ck::command::start(void) {
	if (pid != -1) return -EEXIST;

  pid = fork();
	if (pid == 0) {
		// build the list of args! (It's fine that we leak this, we're execing soon)
  	auto argv = new char *[m_args.size() + 2];
		argv[0] = strdup(m_exe.c_str());
		for (int i = 0; i < m_args.size(); i++) {
			argv[i + 1] = strdup(m_args[i].c_str());
		}
		argv[m_args.size() + 1] = NULL;


		int err = execvp(argv[0], argv);
		perror("exec");

		exit(EXIT_FAILURE);
	}

	return pid;
}

int ck::command::exec(void) {
	int res = start();
  if (res < 0) {
    return -1;
  }
  return wait();
}

int ck::command::wait(void) {
  int stat = -1;
  if (pid != -1) {
		// We are probably losing out on some interesting information if this fails
    waitpid(pid, &stat, 0);
    pid = -1;
  }
  return stat;
}


std::string ck::format(const ck::command &cmd) {
	std::string s = cmd.exe();

	auto argc = cmd.argc();

	auto argv = cmd.argv();
	for (int i = 0; i < argc; i++) {
		s += " "; // add the spacing
		s += argv[i]; // add the argument
	}

	return s;
}

