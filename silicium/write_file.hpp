#ifndef SILICIUM_WRITE_FILE_HPP
#define SILICIUM_WRITE_FILE_HPP

#include <silicium/config.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>

namespace Si
{
	inline void write_file(boost::filesystem::path const &name, char const *data, std::size_t size)
	{
		std::ofstream file(name.string(), std::ios::binary);
		if (!file)
		{
			throw std::runtime_error("Could not open file " + name.string() + " for writing");
		}
		file.write(data, size);
		if (!file)
		{
			throw std::runtime_error("Could not write to file " + name.string());
		}
	}
}

#endif
