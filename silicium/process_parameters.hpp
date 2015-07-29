#ifndef SILICIUM_PROCESS_PARAMETERS_HPP
#define SILICIUM_PROCESS_PARAMETERS_HPP

#include <silicium/source/source.hpp>
#include <silicium/sink/sink.hpp>
#include <silicium/success.hpp>
#include <vector>
#include <string>
#include <boost/filesystem/path.hpp>

namespace Si
{
	struct process_parameters
	{
		boost::filesystem::path executable;

		/// the values for the child's argv[1...]
		std::vector<std::string> arguments;

		/// must be an existing path, otherwise the child cannot launch properly
		boost::filesystem::path current_path;

		/// stdout of the child process will be written to this sink. When nullptr, output is discarded.
		Sink<char, success>::interface *out;

		/// stderr of the child process will be written to this sink. When nullptr, output is discarded.
		Sink<char, success>::interface *err;

		/// provides stdin to the child process. When nullptr, the input will be empty.
		Source<char>::interface *in;

		process_parameters();
	};
	
	inline process_parameters::process_parameters()
		: out(nullptr)
		, err(nullptr)
		, in(nullptr)
	{
	}
}

#endif
