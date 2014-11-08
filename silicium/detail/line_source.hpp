#ifndef SILICIUM_DETAIL_LINE_SOURCE_HPP
#define SILICIUM_DETAIL_LINE_SOURCE_HPP

#include <silicium/source/source.hpp>

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
	}
}

#endif
