#ifndef SILICIUM_SOURCE_HPP
#define SILICIUM_SOURCE_HPP

#include <silicium/override.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/optional.hpp>
#include <boost/cstdint.hpp>

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
		virtual boost::uintmax_t minimum_size() = 0;
		virtual boost::optional<boost::uintmax_t> maximum_size() = 0;
	};

	template <class Element>
	struct memory_source : source<Element>
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

		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE
		{
			return static_cast<boost::uintmax_t>(m_elements.size());
		}

		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE
		{
			return static_cast<boost::uintmax_t>(m_elements.size());
		}

	private:

		boost::iterator_range<Element const *> m_elements;
	};
}

#endif
