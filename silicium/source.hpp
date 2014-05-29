#ifndef SILICIUM_SOURCE_HPP
#define SILICIUM_SOURCE_HPP

#include <silicium/override.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/optional.hpp>
#include <boost/cstdint.hpp>
#include <boost/circular_buffer.hpp>

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

	template <class Element>
	struct buffering_source : source<Element>
	{
		explicit buffering_source(source<Element> &next, std::size_t capacity)
			: m_next(next)
			, m_buffer(capacity)
		{
		}

		virtual boost::iterator_range<Element const *> map_next(std::size_t) SILICIUM_OVERRIDE
		{
			return boost::iterator_range<Element const *>(); //TODO
		}

		virtual Element *copy_next(boost::iterator_range<Element *> destination) SILICIUM_OVERRIDE
		{
			if (m_buffer.empty() && (destination.size() < m_buffer.capacity()))
			{
				m_buffer.resize(m_buffer.capacity());
				auto one = m_buffer.array_one();
				auto copied = m_next.copy_next(boost::make_iterator_range(one.first, one.first + one.second));
				if ((one.first + one.second) == copied)
				{
					auto two = m_buffer.array_two();
					copied = m_next.copy_next(boost::make_iterator_range(two.first, two.first + two.second));
				}
				m_buffer.resize(std::distance(&m_buffer.front(), copied));
			}

			Element *next = destination.begin();

			std::size_t taken_from_buffer = 0;
			for (auto b = m_buffer.begin(); (b != m_buffer.end()) && (next != destination.end()); ++next, ++b, ++taken_from_buffer)
			{
				*next = *b; //TODO move?
			}

			Element * const result = m_next.copy_next(boost::make_iterator_range(next, destination.end()));
			m_buffer.erase_begin(taken_from_buffer);
			return result;
		}

		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE
		{
			return m_buffer.size() + m_next.minimum_size();
		}

		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE
		{
			auto next_max = m_next.maximum_size();
			if (next_max)
			{
				return (*next_max + m_buffer.size());
			}
			return boost::none;
		}

	private:

		source<Element> &m_next;
		boost::circular_buffer<Element> m_buffer;
	};
}

#endif
