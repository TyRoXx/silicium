#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <fstream>
#include <future>

namespace Si
{
	struct process_output
	{
		int return_code;
		//TODO: make stdout asynchronously readable
		std::vector<char> stdout;
	};

	struct process
	{
		std::future<process_output> result;
	};

	struct build_context
	{
		std::function<boost::filesystem::path ()> allocate_temporary_directory;
		std::function<process (std::string const &, std::vector<std::string> const &)> spawn_process;
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
			return m_root / boost::lexical_cast<std::string>(id);
		}

	private:

		boost::filesystem::path m_root;
		boost::uintmax_t m_next_id = 0;
	};

	process spawn_native_process(std::string const &executable, std::vector<std::string> const &arguments)
	{
		throw std::logic_error("not implemented");
	}

	build_context make_native_build_context(boost::filesystem::path temporary_directory_root)
	{
		const auto temporary_dirs = std::make_shared<temporary_directory_allocator>(std::move(temporary_directory_root));
		return build_context
		{
			std::bind(&temporary_directory_allocator::allocate, temporary_dirs),
			spawn_native_process
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
	Si::build_result build(Si::build_context const &context)
	{
		const auto source_dir = context.allocate_temporary_directory();
		const auto source_file = (source_dir / "hello.cpp");
		{
			std::ofstream file(source_file.string());
			file << "#include <iostream>\nint main() { std::cout << \"Hello, world!\\n\"; }\n";
		}
		const auto build_dir = context.allocate_temporary_directory();
		const auto executable_file = build_dir / "hello";
		auto compilation = context.spawn_process("/usr/bin/c++", {source_file.string(), "-o", executable_file.string()});
		const auto compilation_result = compilation.result.get();
		if (compilation_result.return_code != 0)
		{
			return Si::build_failure{"Compilation was not successful"};
		}

		auto testing = context.spawn_process(executable_file.string(), {});
		if (compilation_result.return_code != 0)
		{
			return Si::build_failure{"The built executable returned failure"};
		}

		std::string const expected_output = "Hello, world!\n";
		if (compilation_result.stdout != std::vector<char>(begin(expected_output), end(expected_output)))
		{
			return Si::build_failure{"The built executable did not print the expected text to stdout"};
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

int main()
{
	const auto context = Si::make_native_build_context(boost::filesystem::current_path());
	for (;;)
	{
		const Si::build_result result = build(context);
		result_printer printer(std::cerr);
		boost::apply_visitor(printer, result);

		std::string ignored;
		getline(std::cin, ignored);
	}
}
