#ifndef SILICIUM_DETAIL_LINE_SOURCE_HPP
#define SILICIUM_DETAIL_LINE_SOURCE_HPP

#include <silicium/source.hpp>

namespace Si
{
	namespace detail
	{
		struct line_source SILICIUM_FINAL : Si::source<std::vector<char>>
		{
			line_source()
			{
			}

			explicit line_source(Si::source<char> &next)
				: m_next(&next)
			{
			}

			virtual boost::iterator_range<std::vector<char> const *> map_next(std::size_t) SILICIUM_OVERRIDE
			{
				return boost::iterator_range<std::vector<char> const *>();
			}

			virtual std::vector<char> *copy_next(boost::iterator_range<std::vector<char> *> destination) SILICIUM_OVERRIDE
			{
				assert(m_next);
				auto i = begin(destination);
				for (; i != end(destination); ++i)
				{
					auto &line = *i;
					for (;;)
					{
						auto c = get(*m_next);
						if (!c)
						{
							return &line;
						}
						if (*c == '\r')
						{
							auto lf = get(*m_next);
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
				assert(m_next);
				auto max_chars = m_next->maximum_size();
				//a line can be a single character ('\n')
				return max_chars;
			}

			virtual std::size_t skip(std::size_t count) SILICIUM_OVERRIDE
			{
				std::vector<char> thrown_away;
				for (size_t i = 0; i < count; ++i)
				{
					if (copy_next(boost::make_iterator_range(&thrown_away, &thrown_away + 1)) == &thrown_away)
					{
						return i;
					}
				}
				return count;
			}

		private:

			Si::source<char> *m_next = nullptr;
		};
	}
}

#endif
