#include <silicium/absolute_path.hpp>
#include <silicium/sink/sink.hpp>

//The forward declaration for the function that has to be implemented by the header
//which is #included next.
void configure(
	Si::absolute_path const &module_temporaries,
	Si::absolute_path const &module_permanent,
	Si::absolute_path const &application_source,
	Si::absolute_path const &application_build_dir,
	Si::Sink<char, Si::success>::interface &output
);

//This header is provided by the client application and is expected to define the
//'configure' function (see above for the signature).
#include "cdm.hpp"

#include <silicium/os_string.hpp>
#include <silicium/run_process.hpp>
#include <silicium/sink/ostream_sink.hpp>
#include <silicium/program_options.hpp>
#include <silicium/file_operations.hpp>
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

	if (variables.count("help"))
	{
		LOG(options);
		return 0;
	}

	try
	{
		Si::absolute_path const module_temporaries = Si::temporary_directory(Si::throw_) / *Si::path_segment::create("cdm_modules");
		Si::recreate_directories(module_temporaries, Si::throw_);
		Si::absolute_path const module_permanent = Si::absolute_path::create(module_permanent_argument).or_throw(
			[]{ throw std::invalid_argument("The permanent module cache argument must be an absolute path."); }
		);
		Si::absolute_path const application_source = Si::absolute_path::create(application_source_argument).or_throw(
			[]{ throw std::invalid_argument("The application source argument must be an absolute path."); }
		);
		Si::absolute_path const application_build = Si::absolute_path::create(application_build_argument).or_throw(
			[]{ throw std::invalid_argument("The application build directory argument must be an absolute path."); }
		);
		auto output = Si::Sink<char, Si::success>::erase(Si::ostream_ref_sink(std::cerr));
		::configure(module_temporaries, module_permanent, application_source, application_build, output);
	}
	catch (std::exception const &ex)
	{
		LOG(ex.what());
		return 1;
	}
}
