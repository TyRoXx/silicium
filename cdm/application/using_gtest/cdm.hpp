#include "cdm_gtest.hpp"
#include <silicium/cmake.hpp>

namespace using_gtest
{
	inline void configure(
		Si::absolute_path const &gtest_source,
		Si::absolute_path const &module_temporaries,
		Si::absolute_path const &module_permanent,
		Si::absolute_path const &application_source,
		Si::absolute_path const &application_build_dir,
		Si::sink<char, Si::success> &output
		)
	{
		cdm::gtest_paths const gtest_installed = cdm::install_gtest(gtest_source, module_temporaries, module_permanent, Si::cmake_exe);
		std::vector<Si::os_string> arguments;
		arguments.push_back(Si::os_string(SILICIUM_SYSTEM_LITERAL("-DGTEST_INCLUDE_DIRS=")) + to_os_string(gtest_installed.include));
		arguments.push_back(Si::os_string(SILICIUM_SYSTEM_LITERAL("-DGTEST_LIBRARIES=")) + to_os_string(gtest_installed.library) + SILICIUM_SYSTEM_LITERAL(";") + to_os_string(gtest_installed.library_main));
		arguments.push_back(Si::to_os_string(application_source));
		if (Si::run_process(Si::cmake_exe, arguments, application_build_dir, output).get() != 0)
		{
			throw std::runtime_error("CMake configure failed");
		}
	}
}
