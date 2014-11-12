#ifndef SILICIUM_DETAIL_LINE_SOURCE_HPP
#define SILICIUM_DETAIL_LINE_SOURCE_HPP

#include <silicium/source/source.hpp>
#include <silicium/noexcept_string.hpp>
#include <boost/range/algorithm/find.hpp>

namespace Si
{
	namespace detail
	{
		struct line_source SILICIUM_FINAL : Si::source<std::vector<char>>
		{
			line_source()
				: m_next(nullptr)
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

		private:

			Si::source<char> *m_next;
		};

		template <class CharRange>
		std::pair<noexcept_string, noexcept_string> split_value_line(CharRange const &line)
		{
			auto colon = boost::range::find(line, ':');
			auto second_begin = colon + 1;
			if (*second_begin == ' ')
			{
				++second_begin;
			}
			return std::make_pair(noexcept_string(begin(line), colon), noexcept_string(second_begin, end(line)));
		}
	}
}

#endif
