#ifndef SILICIUM_PROCESS_HPP
#define SILICIUM_PROCESS_HPP

#include <silicium/process_parameters.hpp>

namespace Si
{
	int run_process(process_parameters const &parameters);

	int run_process(
			boost::filesystem::path executable,
			std::vector<std::string> arguments,
			boost::filesystem::path current_path,
			Si::sink<char> &output);
}

#endif
