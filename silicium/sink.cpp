#include <silicium/sink.hpp>
#include <fstream>

namespace Si
{
	std::unique_ptr<flushable_sink<char>> make_file_sink(boost::filesystem::path const &name)
	{
		std::unique_ptr<std::ostream> file(new std::ofstream(name.string(), std::ios::binary));
		if (!*file)
		{
			throw std::runtime_error("Cannot open file for writing: " + name.string());
		}
		return std::unique_ptr<flushable_sink<char>>(new ostream_sink(std::move(file)));
	}
}
