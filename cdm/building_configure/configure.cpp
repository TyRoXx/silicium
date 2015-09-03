#include "configure.hpp"
#include <silicium/file_operations.hpp>
#include <silicium/cmake.hpp>
#include <fstream>

namespace
{
	Si::absolute_path build_configure(
		Si::absolute_path const &application_source,
		Si::absolute_path const &temporary,
		Si::Sink<char, Si::success>::interface &output)
	{
		Si::absolute_path const cdm = Si::parent(
			Si::parent(*Si::absolute_path::create(__FILE__)).or_throw(
				[]{ throw std::runtime_error("Could not find parent directory of this file: " __FILE__); }
			)
		).or_throw(
			[]{ throw std::runtime_error("Could not find the cdm directory"); }
		);
		Si::absolute_path const original_main_cpp = cdm / Si::relative_path("configure_cmdline/main.cpp");
		Si::absolute_path const source = temporary / Si::relative_path("source");
		Si::recreate_directories(source, Si::throw_);
		Si::absolute_path const copied_main_cpp = source / Si::relative_path("main.cpp");
		Si::copy(original_main_cpp, copied_main_cpp, Si::throw_);
		{
			Si::absolute_path const cmakeLists = source / Si::relative_path("CMakeLists.txt");
			std::ofstream cmakeListsFile(cmakeLists.c_str());
			cmakeListsFile << "cmake_minimum_required(VERSION 2.8)\n";
			cmakeListsFile << "project(configure_cmdline_generated)\n";
			cmakeListsFile << "if(UNIX)\n";
			cmakeListsFile << "  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)\n";
			cmakeListsFile << "  if(GCC_VERSION VERSION_GREATER 4.7)\n";
			cmakeListsFile << "    add_definitions(-std=c++1y)\n";
			cmakeListsFile << "  else()\n";
			cmakeListsFile << "    add_definitions(-std=c++0x)\n";
			cmakeListsFile << "  endif()\n";
			cmakeListsFile << "endif()\n";
			cmakeListsFile << "find_package(Boost REQUIRED filesystem coroutine program_options thread context system)\n";
			cmakeListsFile << "include_directories(${SILICIUM_INCLUDE_DIR} ${BOOST_INCLUDE_DIR} ${CDM_CONFIGURE_INCLUDE_DIRS})\n";
			cmakeListsFile << "add_executable(configure main.cpp)\n";
			cmakeListsFile << "target_link_libraries(configure ${Boost_LIBRARIES})\n";
			if (!cmakeListsFile)
			{
				throw std::runtime_error(("Could not generate " + Si::to_utf8_string(cmakeLists)).c_str());
			}
		}
		Si::absolute_path const build = temporary / Si::relative_path("build");
		Si::recreate_directories(build, Si::throw_);
		{
			std::vector<Si::os_string> arguments;
			{
				Si::absolute_path const silicium = Si::parent(cdm).or_throw([] { throw std::runtime_error("Could not find the silicium directory"); });
				arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("-DSILICIUM_INCLUDE_DIR=") + Si::to_os_string(silicium));
			}
			Si::absolute_path const modules = cdm / Si::relative_path("modules");
			arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("-DCDM_CONFIGURE_INCLUDE_DIRS=") + Si::to_os_string(application_source) + SILICIUM_SYSTEM_LITERAL(";") + Si::to_os_string(modules));
			arguments.emplace_back(Si::to_os_string(source));
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
		Si::absolute_path built_executable = build / Si::relative_path("configure");
		return built_executable;
	}

	void run_configure(
		Si::absolute_path const &configure_executable,
		Si::absolute_path const &module_permanent,
		Si::absolute_path const &application_source,
		Si::absolute_path const &application_build_dir,
		Si::Sink<char, Si::success>::interface &output)
	{
		Si::create_directories(module_permanent, Si::throw_);
		Si::create_directories(application_build_dir, Si::throw_);
		std::vector<Si::os_string> arguments;
		arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("-m"));
		arguments.emplace_back(Si::to_os_string(module_permanent));
		arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("-a"));
		arguments.emplace_back(Si::to_os_string(application_source));
		arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("-b"));
		arguments.emplace_back(Si::to_os_string(application_build_dir));
		int const rc = Si::run_process(configure_executable, arguments, application_build_dir, output).get();
		if (rc != 0)
		{
			throw std::runtime_error("Could not configure the application: " + boost::lexical_cast<std::string>(rc));
		}
	}
}

namespace cdm
{
	void do_configure(
		Si::absolute_path const &temporary,
		Si::absolute_path const &module_permanent,
		Si::absolute_path const &application_source,
		Si::absolute_path const &application_build_dir,
		Si::Sink<char, Si::success>::interface &output
	)
	{
		Si::absolute_path const configure_executable = build_configure(application_source, temporary, output);
		run_configure(configure_executable, module_permanent, application_source, application_build_dir, output);
	}
}
