#ifndef VENTURA_PROCESS_PARAMETERS_HPP
#define VENTURA_PROCESS_PARAMETERS_HPP

#include <silicium/source/source.hpp>
#include <silicium/sink/sink.hpp>
#include <silicium/success.hpp>
#include <ventura/absolute_path.hpp>
#include <silicium/os_string.hpp>
#include <vector>
#include <string>
#include <boost/filesystem/path.hpp>

namespace ventura
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
		std::vector<Si::os_string> arguments;

		/// must be an existing path, otherwise the child cannot launch properly
		absolute_path current_path;

		/// stdout of the child process will be written to this sink. When nullptr, output is discarded.
		Si::Sink<char, Si::success>::interface *out;

		/// stderr of the child process will be written to this sink. When nullptr, output is discarded.
		Si::Sink<char, Si::success>::interface *err;

		/// provides stdin to the child process. When nullptr, the input will be empty.
		Si::Source<char>::interface *in;

		std::vector<std::pair<Si::os_char const *, Si::os_char const *>> additional_environment;

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
