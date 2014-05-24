#ifndef SILICIUM_DIRECTORY_BUILDER_HPP
#define SILICIUM_DIRECTORY_BUILDER_HPP

#include <boost/filesystem/path.hpp>
#include <silicium/sink.hpp>

namespace Si
{
	struct directory_builder
	{
		virtual ~directory_builder();
		virtual std::unique_ptr<Si::sink<char>> begin_artifact(std::string const &name) = 0;
		virtual std::unique_ptr<directory_builder> create_subdirectory(std::string const &name) = 0;
	};

	struct filesystem_directory_builder : directory_builder
	{
		explicit filesystem_directory_builder(boost::filesystem::path destination);
		virtual std::unique_ptr<Si::sink<char>> begin_artifact(std::string const &name) override;
		virtual std::unique_ptr<directory_builder> create_subdirectory(std::string const &name) override;

	private:

		boost::filesystem::path m_destination;
	};
}

#endif
