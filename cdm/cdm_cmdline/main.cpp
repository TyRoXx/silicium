#include <silicium/os_string.hpp>
#include <silicium/run_process.hpp>
#include <silicium/sink/ostream_sink.hpp>
#include <silicium/program_options.hpp>
#include <silicium/file_operations.hpp>
#include <iostream>

#define SILICIUM_HAS_CDM (SILICIUM_HAS_RUN_PROCESS && SILICIUM_HAS_PROGRAM_OPTIONS)

#if SILICIUM_HAS_CDM

#ifdef _MSC_VER
#	define SILICIUM_WHILE_FALSE while (0,0)
#else
#	define SILICIUM_WHILE_FALSE while (false)
#endif

#define LOG(...) do { std::cerr << __VA_ARGS__ << '\n'; } SILICIUM_WHILE_FALSE

namespace
{
	void build_configure_command_line(
		Si::absolute_path const &application_source)
	{
		boost::ignore_unused_variable_warning(application_source);
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

int main(int argc, char **argv)
{
	std::string module_permanent_argument;
	std::string application_source_argument;
	std::string application_build_argument;

	boost::program_options::options_description options("options");
	options.add_options()
		("help,h", "produce help message")
		("modules,m", boost::program_options::value(&module_permanent_argument), "absolute path to the permanent module binary cache")
		("application,a", boost::program_options::value(&application_source_argument), "absolute path to the root of your application source code")
		("build,b", boost::program_options::value(&application_build_argument), "absolute path to the CMake build directory of your application")
		;
	boost::program_options::positional_options_description positional;
	boost::program_options::variables_map variables;
	try
	{
		boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(options).positional(positional).run(), variables);
		boost::program_options::notify(variables);
	}
	catch (boost::program_options::error const &ex)
	{
		LOG(options);
		LOG(ex.what());
		return 1;
	}

	if (variables.count("help"))
	{
		LOG(options);
		return 0;
	}

	try
	{
		Si::absolute_path const module_permanent = Si::absolute_path::create(module_permanent_argument).or_throw(
			[]{ throw std::invalid_argument("The permanent module cache argument must be an absolute path."); }
		);
		Si::absolute_path const application_source = Si::absolute_path::create(application_source_argument).or_throw(
			[]{ throw std::invalid_argument("The application source argument must be an absolute path."); }
		);
		Si::absolute_path const application_build = Si::absolute_path::create(application_build_argument).or_throw(
			[]{ throw std::invalid_argument("The application build directory argument must be an absolute path."); }
		);
		build_configure_command_line(application_source);
		run_configure_command_line(module_permanent, application_source, application_build);
	}
	catch (std::exception const &ex)
	{
		LOG(ex.what());
		return 1;
	}
}
#else
int main()
{
	std::cerr << "The compiler or a library is too old.\n";
	return 1;
}
#endif
