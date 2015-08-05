#include <silicium/os_string.hpp>
#include <silicium/run_process.hpp>
#include <silicium/sink/ostream_sink.hpp>
#include <boost/program_options.hpp>
#include <iostream>

#define LOG(...) do { std::cerr << __VA_ARGS__ << '\n'; } while (false)

namespace
{
	Si::absolute_path const seven_zip_exe = *Si::absolute_path::create(
#ifdef _WIN32
		L"C:\\Program Files\\7-Zip\\7z.exe"
#else
		"/usr/bin/7z"
#endif
	);
}

#ifdef _WIN32
#	define SILICIUM_PROGRAM_OPTIONS_NATIVE_VALUE boost::program_options::wvalue
#else
#	define SILICIUM_PROGRAM_OPTIONS_NATIVE_VALUE boost::program_options::value
#endif

int main(int argc, char **argv)
{
	std::vector<Si::os_string> boost_archives;
	Si::os_string install_root_argument;

	boost::program_options::options_description options("options");
	options.add_options()
		("help,h", "produce help message")
		("install-root,i", SILICIUM_PROGRAM_OPTIONS_NATIVE_VALUE(&install_root_argument), "")
		("boost-archive,b", SILICIUM_PROGRAM_OPTIONS_NATIVE_VALUE(&boost_archives), "")
		;
	boost::program_options::positional_options_description positional;
	positional.add("install-root", 1);
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
		Si::optional<Si::absolute_path> const maybe_install_root = Si::absolute_path::create(install_root_argument);
		if (!maybe_install_root)
		{
			LOG("The install root argument is not an absolute path: " << Si::to_utf8_string(install_root_argument));
			return 1;
		}
		Si::create_directories(*maybe_install_root);

		for (Si::os_string const &boost_archive_argument : boost_archives)
		{
			Si::optional<Si::absolute_path> const boost_archive = Si::absolute_path::create(boost_archive_argument);
			if (!boost_archive)
			{
				LOG("The boost archive argument is not an absolute path: " << Si::to_utf8_string(boost_archive_argument));
				return 1;
			}

			Si::optional<Si::path_segment> const boost_archive_name = boost_archive->name();
			if (!boost_archive_name)
			{
				LOG("The boost archive argument is missing a file name: " << Si::to_utf8_string(boost_archive_argument));
				return 1;
			}

			Si::absolute_path const complete_output_directory = *maybe_install_root / *boost_archive_name;
			if (!Si::file_exists(complete_output_directory).get())
			{
				Si::absolute_path const incomplete_output_directory = *maybe_install_root / (*boost_archive_name + *Si::path_segment::create(".incomplete"));

				std::vector<Si::noexcept_string> arguments;
				arguments.push_back("x");
				arguments.push_back("-o" + Si::to_utf8_string(incomplete_output_directory));
				arguments.push_back(Si::to_utf8_string(boost_archive_argument));

				auto command_line_output = Si::virtualize_sink(Si::ostream_ref_sink(std::cout));
				if (Si::run_process(seven_zip_exe.to_boost_path(), arguments, maybe_install_root->to_boost_path(), command_line_output) != 0)
				{
					LOG("Could not run 7zip");
					return 1;
				}

				Si::throw_if_error(Si::rename(incomplete_output_directory, complete_output_directory));
			}
		}
	}
	catch (std::exception const &ex)
	{
		std::cerr << ex.what() << '\n';
		return 1;
	}
}
