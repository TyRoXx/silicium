#ifndef SILICIUM_MEMORY_SOURCE_HPP
#define SILICIUM_MEMORY_SOURCE_HPP

#include <silicium/source.hpp>

namespace Si
{
	template <class Element>
	struct memory_source SILICIUM_FINAL : source<Element>
	{
		memory_source()
		{
		}

		explicit memory_source(boost::iterator_range<Element const *> elements)
			: m_elements(std::move(elements))
		{
		}

		virtual boost::iterator_range<Element const *> map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			(void)size;
			return m_elements;
		}

		virtual Element *copy_next(boost::iterator_range<Element *> destination) SILICIUM_OVERRIDE
		{
			while (!m_elements.empty() && !destination.empty())
			{
				destination.front() = m_elements.front();
				destination.pop_front();
				m_elements.pop_front();
			}
			return destination.begin();
		}

	private:

		boost::iterator_range<Element const *> m_elements;
	};

	template <class Element>
	memory_source<Element> make_container_source(std::vector<Element> const &container)
	{
		return memory_source<Element>({container.data(), container.data() + container.size()});
	}

	template <class Element>
	memory_source<Element> make_container_source(std::basic_string<Element> const &container)
	{
		return memory_source<Element>({container.data(), container.data() + container.size()});
	}
}

#endif
