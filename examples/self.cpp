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
	struct build_context
	{
		std::function<boost::filesystem::path ()> allocate_temporary_directory;
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

	build_context make_native_build_context(boost::filesystem::path temporary_directory_root)
	{
		const auto temporary_dirs = std::make_shared<temporary_directory_allocator>(std::move(temporary_directory_root));
		return build_context
		{
			std::bind(&temporary_directory_allocator::allocate, temporary_dirs)
		};
	}

	struct build_success
	{
	};

	struct build_failure
	{
		std::string short_description;
	};

	typedef boost::variant<build_success, build_failure> build_result;
}

namespace
{
	Si::build_result build(Si::build_context const &context, boost::filesystem::path const silicium_git)
	{
		{
			const auto cmake_result = Si::run_process("/usr/bin/cmake", {silicium_git.string()}, true);
			std::cerr.write(cmake_result.stdout->data(), cmake_result.stdout->size());
			if (cmake_result.exit_status != 0)
			{
				return Si::build_failure{"CMake failed"};
			}
		}

		{
			const auto make_result = Si::run_process("/usr/bin/make", {}, true);
			std::cerr.write(make_result.stdout->data(), make_result.stdout->size());
			if (make_result.exit_status != 0)
			{
				return Si::build_failure{"Make failed"};
			}
		}

		{
			const auto test_result = Si::run_process((boost::filesystem::current_path() / "test/test").string(), {}, true);
			std::cerr.write(test_result.stdout->data(), test_result.stdout->size());
			if (test_result.exit_status != 0)
			{
				return Si::build_failure{"Tests failed"};
			}
		}

		return Si::build_success{};
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

	const auto context = Si::make_native_build_context(boost::filesystem::current_path());
	const Si::build_result result = build(context, silicium_git);
	result_printer printer(std::cerr);
	boost::apply_visitor(printer, result);
	std::cerr << '\n';
}
