#ifndef SILICIUM_SOURCE_REF_HPP
#define SILICIUM_SOURCE_REF_HPP

#include <silicium/source/source.hpp>

namespace Si
{
	template <class Pointee>
	struct ptr_source : source<typename Pointee::element_type>
	{
		typedef typename Pointee::element_type element_type;

		ptr_source()
			: m_next(nullptr)
		{
		}

		explicit ptr_source(Pointee &next)
			: m_next(&next)
		{
		}

		virtual boost::iterator_range<element_type const *> map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			assert(m_next);
			return m_next->map_next(size);
		}

		virtual element_type *copy_next(boost::iterator_range<element_type *> destination) SILICIUM_OVERRIDE
		{
			assert(m_next);
			return m_next->copy_next(destination);
		}

	private:

		Pointee *m_next;
	};

	template <class Pointee>
	auto ref_source(Pointee &next)
	{
		return ptr_source<Pointee>(next);
	}
}

#endif
