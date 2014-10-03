#ifndef SILICIUM_READ_FILE_HPP
#define SILICIUM_READ_FILE_HPP

#include <boost/filesystem/path.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <vector>

namespace Si
{
	inline std::vector<char> read_file(boost::filesystem::path const &file)
	{
		boost::iostreams::mapped_file_source mapped(file);
		return std::vector<char>(mapped.data(), mapped.data() + mapped.size());
	}
}

#endif
