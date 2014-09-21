#include <silicium/process.hpp>
#include <silicium/build_result.hpp>
#include <silicium/directory_allocator.hpp>
#include <silicium/to_unique.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <fstream>
#include <iostream>

namespace
{
	Si::build_result build(Si::directory_allocator const &allocate_temporary_dir)
	{
		const auto source_dir = allocate_temporary_dir();
		const auto source_file = (source_dir / "hello.cpp");
		{
			std::ofstream file(source_file.string());
			file << "#include <iostream>\nint main() { std::cout << \"Hello, world!\\n\"; }\n";
			if (!file)
			{
				throw std::runtime_error("Could not write source file");
			}
		}
		const auto build_dir = allocate_temporary_dir();
		const auto executable_file = build_dir / "hello";
		{
			Si::process_parameters parameters;
			parameters.executable = "/usr/bin/c++";
			parameters.arguments = {source_file.string(), "-o", executable_file.string()};
			parameters.current_path = build_dir;
			const auto compilation_result = Si::run_process(parameters);
			if (compilation_result != 0)
			{
				return Si::build_failure{"Compilation was not successful"};
			}
		}

		std::vector<char> out;
		Si::process_parameters parameters;
		parameters.executable = executable_file.string();
		parameters.current_path = build_dir;
		auto stdout_sink = Si::make_iterator_sink<char>(std::back_inserter(out));
		parameters.out = &stdout_sink;
		const auto testing_result = Si::run_process(parameters);
		if (testing_result != 0)
		{
			return Si::build_failure{"The built executable returned failure"};
		}

		std::string const expected_output = "Hello, world!\n";
		if (out != std::vector<char>(begin(expected_output), end(expected_output)))
		{
			return Si::build_failure{"The built executable did not print the expected text to stdout"};
		}

		return Si::build_success{};
	}

	struct result_printer : boost::static_visitor<>
	{
		explicit result_printer(std::ostream &out)
			: m_out(&out)
		{
		}

		void operator()(Si::build_success) const
		{
			*m_out << "success";
		}

		void operator()(Si::build_failure const &failure) const
		{
			*m_out << "failure: " << failure.short_description;
		}

	private:

		std::ostream *m_out;
	};
}

int main()
{
	Si::incrementing_directory_allocator temporary_dirs(boost::filesystem::current_path());
	Si::directory_allocator const allocate_temporary_dir = std::bind(&Si::incrementing_directory_allocator::allocate, &temporary_dirs);
	const Si::build_result result = build(allocate_temporary_dir);
	result_printer printer(std::cerr);
	boost::apply_visitor(printer, result);
	std::cerr << '\n';
}
