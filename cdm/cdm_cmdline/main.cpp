#include <silicium/os_string.hpp>
#include <silicium/run_process.hpp>
#include <silicium/sink/ostream_sink.hpp>
#include <silicium/program_options.hpp>
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
