#ifndef SILICIUM_WRITE_FILE_HPP
#define SILICIUM_WRITE_FILE_HPP

#include <boost/filesystem/path.hpp>

namespace Si
{
	void write_file(boost::filesystem::path const &file_name, char const *data, std::size_t size);
}

#endif
