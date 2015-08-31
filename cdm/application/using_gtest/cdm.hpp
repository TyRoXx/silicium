#include "cdm_gtest.hpp"
#include <silicium/cmake.hpp>

inline void configure(
	Si::absolute_path const &module_temporaries,
	Si::absolute_path const &module_permanent,
	Si::absolute_path const &application_source,
	Si::absolute_path const &application_build_dir,
	Si::Sink<char, Si::success>::interface &output
	)
{
	Si::optional<Si::absolute_path> const applications = Si::parent(application_source);
	if (!applications)
	{
		throw std::runtime_error("expected the source dir to have a parent");
	}
	Si::optional<Si::absolute_path> const cdm = Si::parent(*applications);
	if (!applications)
	{
		throw std::runtime_error("expected the applications dir to have a parent");
	}
	Si::absolute_path const gtest_source = *cdm / Si::relative_path("original_sources/gtest-1.7.0");
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
