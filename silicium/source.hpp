#ifndef SILICIUM_SOURCE_HPP
#define SILICIUM_SOURCE_HPP

#include <boost/range/iterator_range.hpp>

namespace Si
{
	template <class Element>
	struct source
	{
		virtual ~source()
		{
		}

		virtual boost::iterator_range<Element const *> map_next(std::size_t size) = 0;
		virtual Element *copy_next(boost::iterator_range<Element *> destination) = 0;
	};

	template <class Element>
	struct memory_source : source<Element>
	{
		explicit memory_source(boost::iterator_range<Element const *> elements)
			: m_elements(std::move(elements))
		{
		}

		virtual boost::iterator_range<Element const *> map_next(std::size_t size) override
		{
			return m_elements;
		}

		virtual Element *copy_next(boost::iterator_range<Element *> destination) override
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
}

#endif
