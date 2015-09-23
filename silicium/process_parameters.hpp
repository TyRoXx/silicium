#ifndef SILICIUM_PROCESS_PARAMETERS_HPP
#define SILICIUM_PROCESS_PARAMETERS_HPP

#include <silicium/source/source.hpp>
#include <silicium/sink/sink.hpp>
#include <silicium/success.hpp>
#include <silicium/absolute_path.hpp>
#include <silicium/os_string.hpp>
#include <vector>
#include <string>
#include <boost/filesystem/path.hpp>

namespace Si
{
	enum class environment_inheritance
	{
		inherit,
		no_inherit
	};

	struct process_parameters
	{
		absolute_path executable;

		/// the values for the child's argv[1...]
		std::vector<os_string> arguments;

		/// must be an existing path, otherwise the child cannot launch properly
		absolute_path current_path;

		/// stdout of the child process will be written to this sink. When nullptr, output is discarded.
		Sink<char, success>::interface *out;

		/// stderr of the child process will be written to this sink. When nullptr, output is discarded.
		Sink<char, success>::interface *err;

		/// provides stdin to the child process. When nullptr, the input will be empty.
		Source<char>::interface *in;

		std::vector<std::pair<os_char const *, os_char const *>> additional_environment;

		environment_inheritance inheritance;

		process_parameters();
	};
	
	inline process_parameters::process_parameters()
		: out(nullptr)
		, err(nullptr)
		, in(nullptr)
	    , inheritance(environment_inheritance::inherit)
	{
	}
}

#endif
