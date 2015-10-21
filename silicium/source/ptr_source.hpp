#ifndef SILICIUM_PTR_SOURCE_HPP
#define SILICIUM_PTR_SOURCE_HPP

#include <silicium/source/virtualized_source.hpp>
#include <silicium/to_unique.hpp>

namespace Si
{
	template <class SourcePtr>
	struct ptr_source
	{
		typedef typename std::decay<decltype(*std::declval<SourcePtr>())>::type::element_type element_type;

		ptr_source()
		    : m_ptr(SourcePtr())
		{
		}

		explicit ptr_source(SourcePtr ptr)
		    : m_ptr(std::move(ptr))
		{
		}

#if SILICIUM_COMPILER_GENERATES_MOVES
		SILICIUM_DEFAULT_MOVE(ptr_source)
		SILICIUM_DEFAULT_COPY(ptr_source)
#else
		ptr_source(ptr_source &&other)
		    : m_ptr(std::move(other.m_ptr))
		{
		}

		ptr_source(ptr_source const &other)
		    : m_ptr(other.m_ptr)
		{
		}

		ptr_source &operator=(ptr_source &&other)
		{
			m_ptr = std::move(other.m_ptr);
			return *this;
		}

		ptr_source &operator=(ptr_source const &other)
		{
			m_ptr = other.m_ptr;
			return *this;
		}
#endif

		iterator_range<element_type const *> map_next(std::size_t size)
		{
			return m_ptr->map_next(size);
		}

		element_type *copy_next(iterator_range<element_type *> destination)
		{
			return m_ptr->copy_next(destination);
		}

	private:
		SourcePtr m_ptr;
	};

	template <class Source>
	auto erase_source(Source &&input)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> ptr_source<std::unique_ptr<typename Si::Source<typename std::decay<Source>::type::element_type>::interface>>
#endif
	{
		return ptr_source<
		    std::unique_ptr<typename Si::Source<typename std::decay<Source>::type::element_type>::interface>>(
		    Si::to_unique(virtualize_source(std::forward<Source>(input))));
	}

	template <class Source>
	ptr_source<Source *> ref_source(Source &source)
	{
		return ptr_source<Source *>(&source);
	}
}

#endif
