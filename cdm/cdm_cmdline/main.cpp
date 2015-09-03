#include "building_configure/configure.hpp"
#include <silicium/program_options.hpp>
#include <silicium/file_operations.hpp>
#include <silicium/sink/ostream_sink.hpp>
#include <iostream>

#ifdef _MSC_VER
#	define SILICIUM_WHILE_FALSE while (0,0)
#else
#	define SILICIUM_WHILE_FALSE while (false)
#endif

#define LOG(...) do { std::cerr << __VA_ARGS__ << '\n'; } SILICIUM_WHILE_FALSE

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

	if (variables.count("help") || module_permanent_argument.empty() || application_source_argument.empty() || application_build_argument.empty())
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
		Si::absolute_path const temporary_root = Si::temporary_directory(Si::throw_) / Si::relative_path("cdm_cmdline");
		auto output = Si::Sink<char, Si::success>::erase(Si::ostream_ref_sink(std::cerr));
		cdm::do_configure(temporary_root, module_permanent, application_source, application_build, output);
		LOG("Your application has been configured.");
	}
	catch (std::exception const &ex)
	{
		LOG(ex.what());
		return 1;
	}
}
