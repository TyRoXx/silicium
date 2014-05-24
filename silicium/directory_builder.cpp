#include <silicium/directory_builder.hpp>
#include <boost/filesystem/operations.hpp>

namespace Si
{
	directory_builder::~directory_builder()
	{
	}


	filesystem_directory_builder::filesystem_directory_builder(boost::filesystem::path destination)
		: m_destination(std::move(destination))
	{
	}

	std::unique_ptr<Si::sink<char>> filesystem_directory_builder::begin_artifact(std::string const &name)
	{
		auto const file_name = m_destination / name;
		return make_file_sink(file_name);
	}

	std::unique_ptr<directory_builder> filesystem_directory_builder::create_subdirectory(std::string const &name)
	{
		auto sub = m_destination / name;
		boost::filesystem::create_directories(sub);
		return std::unique_ptr<directory_builder>(new filesystem_directory_builder(sub));
	}
}
