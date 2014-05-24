#ifndef SILICIUM_PROCESS_HPP
#define SILICIUM_PROCESS_HPP

#include <silicium/sink.hpp>
#include <vector>
#include <string>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>

namespace Si
{
	struct process_output
	{
		int exit_status;
		//TODO: make stdout asynchronously readable
		boost::optional<std::vector<char>> stdout;
	};

	process_output run_process(std::string executable, std::vector<std::string> arguments, boost::filesystem::path const &current_path, bool dump_stdout);

	template <class Element>
	struct source
	{
		virtual ~source()
		{
		}

		virtual boost::iterator_range<Element const *> map_next(std::size_t size) = 0;
		virtual Element *copy_next(boost::iterator_range<Element *> destination) = 0;
	};

	struct process_parameters
	{
		std::string executable;
		std::vector<std::string> arguments;
		boost::filesystem::path current_path;
		std::unique_ptr<sink<char>> stdout;
		std::unique_ptr<sink<char>> stderr;
		std::unique_ptr<source<char>> stdin;
	};

	int run_process(process_parameters const &parameters);
}

#endif
