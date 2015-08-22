#ifndef SILICIUM_CONTAINER_BUFFER_HPP
#define SILICIUM_CONTAINER_BUFFER_HPP

#include <silicium/success.hpp>
#include <silicium/iterator_range.hpp>
#include <algorithm>

namespace Si
{
	template <class ContiguousContainer>
	struct container_buffer
	{
		typedef typename ContiguousContainer::value_type element_type;
		typedef success error_type;

		container_buffer()
			: m_destination(nullptr)
			, m_committed(0)
		{
		}

		explicit container_buffer(ContiguousContainer &destination)
			: m_destination(&destination)
			, m_committed(destination.size())
		{
		}

		iterator_range<element_type *> make_append_space(std::size_t size)
		{
			assert(m_destination);
			std::size_t remaining_capacity = m_destination->max_size() - m_committed;
			std::size_t growth = std::min(size, remaining_capacity);
			std::size_t new_size = m_committed + growth;
			m_destination->resize(new_size);
			auto begin = m_destination->data() + m_committed;
			return make_iterator_range(begin, begin + growth);
		}

		error_type flush_append_space()
		{
			assert(m_destination);
			m_committed = m_destination->size();
			return success();
		}

	private:

		ContiguousContainer *m_destination;
		std::size_t m_committed;
	};

	template <class ContiguousContainer>
	auto make_container_buffer(ContiguousContainer &destination)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> container_buffer<ContiguousContainer>
#endif
	{
		return container_buffer<ContiguousContainer>(destination);
	}
}

#endif
