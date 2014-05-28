#ifndef SILICIUM_DIRECTORY_ALLOCATOR_HPP
#define SILICIUM_DIRECTORY_ALLOCATOR_HPP

#include <functional>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

namespace Si
{
	typedef std::function<boost::filesystem::path ()> directory_allocator;

	struct incrementing_directory_allocator
	{
		explicit incrementing_directory_allocator(boost::filesystem::path root);
		boost::filesystem::path allocate();

	private:

		boost::filesystem::path m_root;
		boost::uintmax_t m_next_id;
	};
}

#endif
