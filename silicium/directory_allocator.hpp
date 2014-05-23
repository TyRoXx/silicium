#ifndef SILICIUM_DIRECTORY_ALLOCATOR_HPP
#define SILICIUM_DIRECTORY_ALLOCATOR_HPP

#include <functional>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

namespace Si
{
	typedef std::function<boost::filesystem::path ()> directory_allocator;

	struct temporary_directory_allocator
	{
		explicit temporary_directory_allocator(boost::filesystem::path root)
			: m_root(std::move(root))
		{
		}

		boost::filesystem::path allocate()
		{
			const auto id = m_next_id++;
			auto directory = m_root / boost::lexical_cast<std::string>(id);
			boost::filesystem::create_directories(directory);
			return directory;
		}

	private:

		boost::filesystem::path m_root;
		boost::uintmax_t m_next_id = 0;
	};
}

#endif
