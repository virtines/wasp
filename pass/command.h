#pragma once

#include <string>
#include <vector>
#include <unistd.h>

namespace ck {
  class command {
    pid_t pid = -1;
		std::string m_exe;
		std::vector<std::string> m_args;

   public:
    command(std::string exe);
    template <typename... T>
    inline command(std::string exe, T... args) : m_exe(exe) {
      this->args(args...);
    }

    ~command(void);

    template <typename T>
    void args(T t) {
      arg(t);
    }

    template <typename T, typename... Rest>
    void args(T t, Rest... rest) {
      arg(t);
      args(rest...);
    }

    void arg(std::string);


    inline const std::string &exe(void) const { return m_exe; }
    inline int argc(void) const { return m_args.size(); }
    inline std::vector<std::string> argv(void) const { return m_args; }

    // start the command
    int start(void);

    // starts then waits
    int exec(void);

    // waits for the started command to exit
    int wait(void);
  };

	std::string format(const ck::command &cmd);


  template <typename... T>
	int exec(T... args) {
		return ck::command(args...).exec();
	}
}  // namespace ck
