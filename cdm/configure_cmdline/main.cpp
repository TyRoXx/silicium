#include "cdm.hpp"
#include <silicium/os_string.hpp>
#include <silicium/run_process.hpp>
#include <silicium/sink/ostream_sink.hpp>
#include <silicium/program_options.hpp>
#include <iostream>

#ifdef _MSC_VER
#	define SILICIUM_WHILE_FALSE while (0,0)
#else
#	define SILICIUM_WHILE_FALSE while (false)
#endif

#define LOG(...) do { std::cerr << __VA_ARGS__ << '\n'; } SILICIUM_WHILE_FALSE

namespace
{
}

#ifdef _WIN32
#	define SILICIUM_PROGRAM_OPTIONS_NATIVE_VALUE boost::program_options::wvalue
#else
#	define SILICIUM_PROGRAM_OPTIONS_NATIVE_VALUE boost::program_options::value
#endif

int main(int argc, char **argv)
{
	boost::program_options::options_description options("options");
	options.add_options()
		("help,h", "produce help message")
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

		Si::absolute_path const module_temporaries;
		Si::absolute_path const module_permanent;
		Si::absolute_path const application_source;
		Si::absolute_path const application_build_dir;
		auto output = Si::Sink<char, Si::success>::erase(Si::ostream_ref_sink(std::cerr));
		::configure(module_temporaries, module_permanent, application_source, application_build_dir, output);
	}
	catch (std::exception const &ex)
	{
		LOG(ex.what());
		return 1;
	}
}
