#ifndef SILICIUM_PROCESS_PARAMETERS_HPP
#define SILICIUM_PROCESS_PARAMETERS_HPP

#include <silicium/source.hpp>
#include <silicium/sink.hpp>
#include <vector>
#include <string>
#include <boost/filesystem/path.hpp>

namespace Si
{
	struct process_parameters
	{
		boost::filesystem::path executable;
		std::vector<std::string> arguments;
		boost::filesystem::path current_path;
		sink<char> *out;
		sink<char> *err;
		source<char> *in;

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
