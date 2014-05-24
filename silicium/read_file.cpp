#include <silicium/read_file.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

namespace Si
{
	std::vector<char> read_file(boost::filesystem::path const &file)
	{
		boost::iostreams::mapped_file_source mapped(file);
		return std::vector<char>(mapped.data(), mapped.data() + mapped.size());
	}
}
