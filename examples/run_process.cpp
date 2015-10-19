#include <ventura/run_process.hpp>
#include <ventura/file_operations.hpp>
#include <silicium/sink/function_sink.hpp>

int main(int argc, char **argv)
{
	boost::ignore_unused_variable_warning(argc);
#if VENTURA_HAS_RUN_PROCESS
	ventura::process_parameters parameters;
	parameters.executable = *ventura::absolute_path::create("/usr/bin/file");
	parameters.current_path = ventura::get_current_working_directory(Si::throw_);
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
	return ventura::run_process(parameters);
#else
	boost::ignore_unused_variable_warning(argv);
	std::cerr << "This example requires Si::run_process to be available\n";
#endif
}
