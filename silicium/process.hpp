#ifndef SILICIUM_PROCESS_HPP
#define SILICIUM_PROCESS_HPP

#include <silicium/source.hpp>
#include <silicium/sink.hpp>
#include <vector>
#include <string>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

namespace Si
{
	struct process_parameters
	{
		boost::filesystem::path executable;
		std::vector<std::string> arguments;
		boost::filesystem::path current_path;
		sink<char> *stdout;
		sink<char> *stderr;
		source<char> *stdin;

		process_parameters();
	};

	int run_process(process_parameters const &parameters);

	int run_process(
			boost::filesystem::path executable,
			std::vector<std::string> arguments,
			boost::filesystem::path current_path,
			Si::sink<char> &output);
}

#endif
