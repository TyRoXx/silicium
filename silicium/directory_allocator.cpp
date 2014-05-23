#include <silicium/directory_allocator.hpp>

namespace Si
{
	incrementing_directory_allocator::incrementing_directory_allocator(boost::filesystem::path root)
		: m_root(std::move(root))
	{
	}

	boost::filesystem::path incrementing_directory_allocator::allocate()
	{
		const auto id = m_next_id++;
		auto directory = m_root / boost::lexical_cast<std::string>(id);
		boost::filesystem::create_directories(directory);
		return directory;
	}
}
