#ifndef SILICIUM_PROCESS_HPP
#define SILICIUM_PROCESS_HPP

#include <vector>
#include <future>
#include <string>

namespace Si
{
	struct process_output
	{
		int return_code;
		//TODO: make stdout asynchronously readable
		std::vector<char> stdout;
	};

	struct process
	{
		std::future<process_output> result;
	};

	process spawn_native_process(std::string const &executable, std::vector<std::string> const &arguments)
	{
		throw std::logic_error("not implemented");
	}
}

#endif
