#ifndef SILICIUM_PROCESS_HPP
#define SILICIUM_PROCESS_HPP

#include <vector>
#include <string>
#include <algorithm>
#include <boost/system/system_error.hpp>

#ifdef __linux__
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace Si
{
	struct process_output
	{
		int exit_status;
		//TODO: make stdout asynchronously readable
		std::vector<char> stdout;
	};

#ifdef __linux__
	process_output run_process(std::string executable, std::vector<std::string> arguments)
	{
		std::vector<char *> argument_pointers;
		argument_pointers.emplace_back(&executable[0]);
		std::transform(begin(arguments), end(arguments), std::back_inserter(argument_pointers), [](std::string &arg)
		{
			return &arg[0];
		});
		argument_pointers.emplace_back(nullptr);

		pid_t const forked = fork();
		if (forked < 0)
		{
			throw boost::system::system_error(errno, boost::system::system_category());
		}

		//child?
		if (forked == 0)
		{
			execv(executable.c_str(), argument_pointers.data());

			//kill the process in case execv fails
			std::abort();
		}

		//parent
		else
		{
			int status = 0;
			if (waitpid(forked, &status, 0) < 0)
			{
				throw boost::system::system_error(errno, boost::system::system_category());
			}

			int const exit_status = WEXITSTATUS(status);
			return process_output{exit_status, {/*TODO*/}};
		}
	}
#endif
}

#endif
