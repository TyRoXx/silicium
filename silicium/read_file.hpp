#ifndef SILICIUM_READ_FILE_HPP
#define SILICIUM_READ_FILE_HPP

#include <boost/filesystem/path.hpp>
#include <vector>

namespace Si
{
	std::vector<char> read_file(boost::filesystem::path const &file);
}

#endif
