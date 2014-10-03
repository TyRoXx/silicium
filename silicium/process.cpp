#include <silicium/process.hpp>
#include <silicium/to_unique.hpp>

#ifdef _WIN32
#	include <silicium/win32/process.hpp>
#endif

#ifdef __linux__
#	include <silicium/linux/process.hpp>
#endif

namespace Si
{
	int run_process(
			boost::filesystem::path executable,
			std::vector<std::string> arguments,
			boost::filesystem::path current_path,
			Si::sink<char> &output)
	{
		Si::process_parameters parameters;
		parameters.executable = std::move(executable);
		parameters.arguments = std::move(arguments);
		parameters.current_path = std::move(current_path);
		parameters.out = &output;
		return Si::run_process(parameters);
	}
}
