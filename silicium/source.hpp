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

	template <class Element>
	boost::optional<Element> get(Si::source<Element> &from)
	{
		Element result;
		if (&result == from.copy_next(boost::make_iterator_range(&result, &result + 1)))
		{
			return boost::none;
		}
		return std::move(result);
	}

	struct line_source : Si::source<std::vector<char>>
	{
		explicit line_source(Si::source<char> &next)
			: m_next(next)
		{
		}

		virtual boost::iterator_range<std::vector<char> const *> map_next(std::size_t) SILICIUM_OVERRIDE
		{
			return boost::iterator_range<std::vector<char> const *>();
		}

		virtual std::vector<char> *copy_next(boost::iterator_range<std::vector<char> *> destination) SILICIUM_OVERRIDE
		{
			auto i = begin(destination);
			for (; i != end(destination); ++i)
			{
				auto &line = *i;
				for (;;)
				{
					auto c = get(m_next);
					if (!c)
					{
						return &line;
					}
					if (*c == '\r')
					{
						auto lf = get(m_next);
						if (!lf)
						{
							return &line;
						}
						if (*lf == '\n')
						{
							break;
						}
					}
					if (*c == '\n')
					{
						break;
					}
					line.emplace_back(*c);
				}
			}
			return i;
		}

		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE
		{
			return 0;
		}

		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE
		{
			auto max_chars = m_next.maximum_size();
			//a line can be a single character ('\n')
			return max_chars;
		}

	private:

		Si::source<char> &m_next;
	};
}

#endif
