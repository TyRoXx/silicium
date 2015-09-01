#include "building_configure.hpp"
#include <silicium/file_operations.hpp>
#include <silicium/sink/ostream_sink.hpp>
#include <silicium/cmake.hpp>

namespace cdm
{
	void build_configure_command_line(
		Si::absolute_path const &application_source,
		Si::absolute_path const &temporary)
	{
		Si::absolute_path const cdm = Si::parent(*Si::absolute_path::create(__FILE__)).or_throw([]{ throw std::runtime_error("Could not find parent directory of this file: " __FILE__); });
		Si::absolute_path const original_main_cpp = cdm / Si::relative_path("configure_cmdline/main.cpp");
		Si::absolute_path const source = temporary / Si::relative_path("source");
		Si::absolute_path const copied_main_cpp = source / Si::relative_path("main.cpp");
		Si::copy(original_main_cpp, copied_main_cpp, Si::throw_);
		{
			Si::absolute_path const cmakeLists = source / Si::relative_path("CMakeLists.txt");
			std::ofstream cmakeListsFile(cmakeLists.c_str());
			cmakeListsFile << "project(configure_cmdline_generated)\n";
			cmakeListsFile << "find_package(Boost REQUIRED filesystem system)\n";
			cmakeListsFile << "include_directories(${SILICIUM_INCLUDE_DIR} ${BOOST_INCLUDE_DIR} ${CDM_CONFIGURE_INCLUDE_DIR})\n";
			cmakeListsFile << "add_executable(configure main.cpp)\n";
			cmakeListsFile << "target_link_libraries(configure ${Boost_LIBRARIES})\n";
			if (!cmakeListsFile)
			{
				throw std::runtime_error(("Could not generate " + Si::to_utf8_string(cmakeLists)).c_str());
			}
		}
		Si::absolute_path const build = temporary / Si::relative_path("build");
		auto output = Si::Sink<char, Si::success>::erase(Si::ostream_ref_sink(std::cerr));
		{
			std::vector<Si::os_string> arguments;
			{
				Si::absolute_path const silicium = Si::parent(cdm).or_throw([] { throw std::runtime_error("Could not find the silicium directory"); });
				arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("-DSILICIUM_INCLUDE_DIR=") + Si::to_os_string(silicium));
			}
			arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("-DCDM_CONFIGURE_INCLUDE_DIR=") + Si::to_os_string(application_source));
			if (Si::run_process(Si::cmake_exe, arguments, build, output).get() != 0)
			{
				throw std::runtime_error("Could not CMake-configure the cdm configure executable");
			}
		}
		{
			std::vector<Si::os_string> arguments;
			arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("--build"));
			arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("."));
			if (Si::run_process(Si::cmake_exe, arguments, build, output).get() != 0)
			{
				throw std::runtime_error("Could not CMake --build the cdm configure executable");
			}
		}
		throw std::logic_error("not implemented");
	}

	void run_configure_command_line(
		Si::absolute_path const &module_permanent,
		Si::absolute_path const &application_source,
		Si::absolute_path const &application_build_dir)
	{
		boost::ignore_unused_variable_warning(module_permanent);
		boost::ignore_unused_variable_warning(application_source);
		boost::ignore_unused_variable_warning(application_build_dir);
		throw std::logic_error("not implemented");
	}
}
