#ifndef SILICIUM_REACTIVE_PROCESS_HPP
#define SILICIUM_REACTIVE_PROCESS_HPP

#include <reactive/ptr_observable.hpp>
#include <boost/filesystem/path.hpp>
#include <vector>

namespace rx
{
	struct process
	{
		unique_observable<int> exit_code;
		unique_observable<char> error;
		unique_observable<char> output;
	};

	inline process launch_process(boost::filesystem::path executable, std::vector<std::string> arguments, observable<char> &input)
	{
		throw std::logic_error("not implemented");
	}
}

#endif
