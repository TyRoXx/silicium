#include <silicium/os_string.hpp>
#include <silicium/run_process.hpp>
#include <silicium/sink/ostream_sink.hpp>
#include <silicium/file_operations.hpp>
#include <silicium/program_options.hpp>
#include <iostream>

#define SILICIUM_HAS_DEPENDENCY_BUILDER (SILICIUM_HAS_RUN_PROCESS && SILICIUM_HAS_PROGRAM_OPTIONS)

#if SILICIUM_HAS_DEPENDENCY_BUILDER

#ifdef _MSC_VER
#	define SILICIUM_WHILE_FALSE while (0,0)
#else
#	define SILICIUM_WHILE_FALSE while (false)
#endif

#define LOG(...) do { std::cerr << __VA_ARGS__ << '\n'; } SILICIUM_WHILE_FALSE

namespace
{
	Si::absolute_path const seven_zip_exe = *Si::absolute_path::create(
#ifdef _WIN32
		L"C:\\Program Files\\7-Zip\\7z.exe"
#else
		"/usr/bin/7z"
#endif
	);

	SILICIUM_USE_RESULT
	Si::optional<Si::absolute_path> extract_safely(Si::absolute_path const &install_root, Si::absolute_path const &boost_archive)
	{
		Si::optional<Si::path_segment> const boost_archive_name = boost_archive.name();
		if (!boost_archive_name)
		{
			LOG("The boost archive argument is missing a file name: " << Si::to_utf8_string(boost_archive));
			return Si::none;
		}

		Si::absolute_path const complete_output_directory = install_root / *boost_archive_name;
		if (Si::file_exists(complete_output_directory, Si::throw_))
		{
			return complete_output_directory;
		}

		Si::absolute_path const incomplete_output_directory = install_root / (*boost_archive_name + *Si::path_segment::create(".incomplete"));

		std::vector<Si::noexcept_string> arguments;
		arguments.push_back("x");
		arguments.push_back("-o" + Si::to_utf8_string(incomplete_output_directory));
		arguments.push_back(Si::to_utf8_string(boost_archive));

		auto command_line_output = Si::virtualize_sink(Si::ostream_ref_sink(std::cout));
		if (Si::run_process(seven_zip_exe.to_boost_path(), arguments, install_root.to_boost_path(), command_line_output) != 0)
		{
			LOG("Could not run 7zip");
			return Si::none;
		}

		Si::throw_if_error(Si::rename(incomplete_output_directory, complete_output_directory));
		return complete_output_directory;
	}

	SILICIUM_USE_RESULT
	Si::optional<Si::absolute_path> find_actual_boost_root(Si::absolute_path const &boost_extraction_dir)
	{
		for (boost::filesystem::directory_iterator i(boost_extraction_dir.to_boost_path()); i != boost::filesystem::directory_iterator(); ++i)
		{
			switch (i->status().type())
			{
			case boost::filesystem::directory_file:
				{
					return Si::absolute_path::create(i->path());
				}

			default:
				break;
			}
		}
		return Si::none;
	}

	enum class bootstrap_result
	{
		success,
		failure
	};

	SILICIUM_USE_RESULT
	bootstrap_result bootstrap(Si::absolute_path const &boost_extraction_dir)
	{
		Si::optional<Si::absolute_path> const actual_boost_root = find_actual_boost_root(boost_extraction_dir);
		if (!actual_boost_root)
		{
			LOG("Did not find a Boost root in the extraction directory: " << Si::to_utf8_string(boost_extraction_dir));
			return bootstrap_result::failure;
		}

		std::vector<Si::noexcept_string> arguments;
		arguments.push_back(Si::to_utf8_string(*actual_boost_root / *Si::path_segment::create("bootstrap.sh")));

		auto command_line_output = Si::virtualize_sink(Si::ostream_ref_sink(std::cout));
		if (Si::run_process("/usr/bin/env", arguments, actual_boost_root->to_boost_path(), command_line_output) != 0)
		{
			LOG("Boost bootstrapping failed");
			return bootstrap_result::failure;
		}

		return bootstrap_result::success;
	}
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

			Si::optional<Si::absolute_path> const boost_extraction_dir = extract_safely(*maybe_install_root, *boost_archive);
			if (!boost_extraction_dir)
			{
				return 1;
			}

			switch (bootstrap(*boost_extraction_dir))
			{
			case bootstrap_result::failure:
				return 1;

			case bootstrap_result::success:
				break;
			}
		}
	}
	catch (std::exception const &ex)
	{
		std::cerr << ex.what() << '\n';
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
