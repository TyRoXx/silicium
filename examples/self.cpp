#include <silicium/process.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <fstream>
#include <future>

namespace Si
{
	struct build_success
	{
	};

	struct build_failure
	{
		std::string short_description;
	};

	typedef boost::variant<build_success, build_failure> build_result;

	struct report
	{
		std::function<void (std::string, std::vector<char>)> add_artifact;
	};

	typedef std::function<void (build_result)> report_finalizer;

	struct build_context
	{
		std::function<boost::filesystem::path ()> allocate_temporary_directory;
		std::function<std::pair<report, report_finalizer> (std::string)> create_report;
	};

	struct temporary_directory_allocator
	{
		explicit temporary_directory_allocator(boost::filesystem::path root)
			: m_root(std::move(root))
		{
		}

		boost::filesystem::path allocate()
		{
			const auto id = m_next_id++;
			auto directory = m_root / boost::lexical_cast<std::string>(id);
			boost::filesystem::create_directories(directory);
			return directory;
		}

	private:

		boost::filesystem::path m_root;
		boost::uintmax_t m_next_id = 0;
	};
}

namespace
{
	Si::build_result build_configuration(
			Si::report const &result,
			boost::filesystem::path const &build_directory,
			boost::filesystem::path const &source,
			std::string const &build_type)
	{
		{
			const auto cmake_result = Si::run_process("/usr/bin/cmake", {source.string(), ("-DCMAKE_BUILD_TYPE=" + build_type)}, build_directory, true);
			result.add_artifact("CMake", *cmake_result.stdout);
			if (cmake_result.exit_status != 0)
			{
				return Si::build_failure{"CMake failed"};
			}
		}

		{
			const auto make_result = Si::run_process("/usr/bin/make", {}, build_directory, true);
			result.add_artifact("Make", *make_result.stdout);
			if (make_result.exit_status != 0)
			{
				return Si::build_failure{"Make failed"};
			}
		}

		{
			const auto test_result = Si::run_process((build_directory / "test/test").string(), {}, build_directory, true);
			result.add_artifact("Test", *test_result.stdout);
			if (test_result.exit_status != 0)
			{
				return Si::build_failure{"Test failed"};
			}
		}

		return Si::build_success{};
	}

	void build(Si::build_context const &context, boost::filesystem::path const silicium_git)
	{
		boost::filesystem::path const &build_root = context.allocate_temporary_directory();
		std::vector<std::string> const build_types = {"DEBUG", "RELEASE"};
		for (auto &build_type : build_types)
		{
			auto const build_dir = build_root / build_type;
			boost::filesystem::create_directories(build_dir);

			auto const report_and_finalizer = context.create_report(build_type);
			auto const result = build_configuration(report_and_finalizer.first, build_dir, silicium_git, build_type);
			report_and_finalizer.second(result);
		}
	}

	struct result_printer : boost::static_visitor<>
	{
		explicit result_printer(std::ostream &out)
			: m_out(out)
		{
		}

		void operator()(Si::build_success) const
		{
			m_out << "success";
		}

		void operator()(Si::build_failure const &failure) const
		{
			m_out << "failure: " << failure.short_description;
		}

	private:

		std::ostream &m_out;
	};
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		return 1;
	}
	boost::filesystem::path const silicium_git = argv[1];

	const auto temporary_dirs = std::make_shared<Si::temporary_directory_allocator>(boost::filesystem::current_path());
	auto const report_root = temporary_dirs->allocate();
	Si::build_context context
	{
		std::bind(&Si::temporary_directory_allocator::allocate, temporary_dirs),
		[report_root](std::string name)
		{
			auto const report_dir = report_root / name;
			boost::filesystem::create_directories(report_dir);
			Si::report r
			{
				[report_dir](std::string name, std::vector<char> content)
				{
					auto const file_name = (report_dir / name).string();
					std::ofstream file(file_name, std::ios::binary);
					file.write(content.data(), content.size());
					if (!file)
					{
						throw std::runtime_error("Could not write file " + file_name);
					}
				}
			};
			auto const handle_result = [](Si::build_result const &result)
			{
				result_printer printer(std::cerr);
				boost::apply_visitor(printer, result);
				std::cerr << '\n';
			};
			return std::make_pair(std::move(r), handle_result);
		}
	};

	build(context, silicium_git);
}
