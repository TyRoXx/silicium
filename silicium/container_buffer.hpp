#ifndef SILICIUM_CONTAINER_BUFFER_HPP
#define SILICIUM_CONTAINER_BUFFER_HPP

#include <silicium/sink.hpp>

namespace Si
{
	template <class ContiguousContainer>
	struct container_buffer : buffer<typename ContiguousContainer::value_type, void>
	{
		typedef typename ContiguousContainer::value_type element_type;
		typedef void error_type;

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

		virtual boost::iterator_range<element_type *> make_append_space(std::size_t size) SILICIUM_OVERRIDE
		{
			assert(m_destination);
			auto remaining_capacity = m_destination->max_size() - m_committed;
			auto growth = std::min(size, remaining_capacity);
			auto new_size = m_committed + growth;
			m_destination->resize(new_size);
			auto begin = m_destination->data() + m_committed;
			return boost::make_iterator_range(begin, begin + growth);
		}

		virtual error_type flush_append_space() SILICIUM_OVERRIDE
		{
			assert(m_destination);
			m_committed = m_destination->size();
		}

	private:

		ContiguousContainer *m_destination;
		std::size_t m_committed;
	};

	template <class ContiguousContainer>
	auto make_container_buffer(ContiguousContainer &destination)
	{
		return container_buffer<ContiguousContainer>(destination);
	}
}

#endif
