#include <silicium/run_process.hpp>
#include <silicium/sink/function_sink.hpp>

int main(int argc, char **argv)
{
	boost::ignore_unused_variable_warning(argc);
	Si::process_parameters parameters;
	parameters.executable = "/usr/bin/file";
	parameters.current_path = boost::filesystem::current_path();
	parameters.arguments.emplace_back(argv[0]);
	auto output = Si::Sink<char, Si::success>::erase(
		Si::make_function_sink<char>(
			[](Si::iterator_range<char const *> data) -> Si::success
			{
				std::cout.write(data.begin(), data.size());
				return {};
			}
		)
	);
	parameters.out = &output;
	std::cerr << "This executable is:\n";
	return Si::run_process(parameters);
}
