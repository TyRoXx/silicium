#include <silicium/process.hpp>
#include <silicium/build_result.hpp>
#include <silicium/directory_allocator.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <fstream>
#include <iostream>

namespace Si
{
	struct report
	{
		std::function<void (std::string, std::vector<char>)> add_artifact;
	};

	typedef std::function<void (build_result)> report_finalizer;
	typedef std::function<std::pair<report, report_finalizer> (std::string)> report_creator;
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
			result.add_artifact("cmake.log", *cmake_result.stdout);
			if (cmake_result.exit_status != 0)
			{
				return Si::build_failure{"CMake failed"};
			}
		}

		{
			const auto make_result = Si::run_process("/usr/bin/make", {}, build_directory, true);
			result.add_artifact("make.log", *make_result.stdout);
			if (make_result.exit_status != 0)
			{
				return Si::build_failure{"Make failed"};
			}
		}

		{
			const auto test_result = Si::run_process((build_directory / "test/test").string(), {}, build_directory, true);
			result.add_artifact("test.log", *test_result.stdout);
			if (test_result.exit_status != 0)
			{
				return Si::build_failure{"Test failed"};
			}
		}

		return Si::build_success{};
	}

	void build(Si::report_creator const &create_report, boost::filesystem::path const &build_root, boost::filesystem::path const &silicium_git)
	{
		std::vector<std::string> const build_types = {"DEBUG", "RELEASE"};
		for (auto &build_type : build_types)
		{
			auto const build_dir = build_root / build_type;
			boost::filesystem::create_directories(build_dir);

			auto const report_and_finalizer = create_report(build_type);
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

	std::pair<Si::report, Si::report_finalizer> create_simple_file_report(boost::filesystem::path const &parent, std::string name)
	{
		auto const report_dir = parent / name;
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
		auto const handle_result = [report_dir](Si::build_result const &result)
		{
			auto const report_file_name = (report_dir / "report.txt").string();
			std::ofstream file(report_file_name);
			{
				result_printer printer(file);
				boost::apply_visitor(printer, result);
				file << '\n';
			}
			if (!file)
			{
				throw std::runtime_error("Could not write report to " + report_file_name);
			}
		};
		return std::make_pair(std::move(r), handle_result);
	}
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		return 1;
	}
	boost::filesystem::path const silicium_git = argv[1];

	Si::incrementing_directory_allocator temporary_dirs(boost::filesystem::current_path());
	Si::directory_allocator const allocate_temporary_dir = std::bind(&Si::incrementing_directory_allocator::allocate, &temporary_dirs);
	auto const report_root = allocate_temporary_dir();
	Si::report_creator const create_port = [report_root](std::string name) { return create_simple_file_report(report_root, std::move(name)); };

	build(create_port, allocate_temporary_dir(), silicium_git);
}
