#ifndef SILICIUM_ENUMERATING_SOURCE_HPP
#define SILICIUM_ENUMERATING_SOURCE_HPP

#include <silicium/source/source.hpp>
#include <silicium/exchange.hpp>

namespace Si
{
	namespace detail
	{
		template <class Range>
		struct value_type
		{
			typedef typename std::iterator_traits<
				typename std::decay<
					decltype(std::declval<Range>().begin())
				>::type
			>::value_type type;
		};
	}

	template <class RangeSource>
	struct enumerating_source : source<typename detail::value_type<typename RangeSource::element_type>::type>
	{
		typedef typename detail::value_type<typename RangeSource::element_type>::type element_type;
		typedef typename RangeSource::element_type range_type;

		enumerating_source()
		{
		}

		explicit enumerating_source(RangeSource input)
			: m_input(std::move(input))
		{
		}

		virtual iterator_range<element_type const *> map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			if (!m_rest.empty())
			{
				iterator_range<element_type const *> result(m_rest.begin(), m_rest.end());
				m_rest = range_type();
				return result;
			}
			boost::ignore_unused_variable_warning(size);
			auto element = Si::get(m_input);
			if (!element)
			{
				return {};
			}
			return iterator_range<element_type const *>(element->begin(), element->end());
		}

		virtual element_type *copy_next(iterator_range<element_type *> destination) SILICIUM_OVERRIDE
		{
			element_type *copied = destination.begin();
			for (;;)
			{
				std::size_t const requested = std::distance(copied, destination.end());
				if (requested == 0)
				{
					break;
				}
				if (m_rest.empty())
				{
					auto element = Si::get(m_input);
					if (!element)
					{
						break;
					}
					m_rest = std::move(*element);
				}
				std::size_t const provided = m_rest.size();
				if (provided == 0)
				{
					break;
				}
				auto const copyable = std::min(requested, provided);
				copied = std::copy_n(m_rest.begin(), copyable, copied);
				m_rest = range_type(m_rest.begin() + copyable, m_rest.end());
			}
			return copied;
		}

	private:

		RangeSource m_input;
		range_type m_rest;
	};

	template <class RangeSource>
	auto make_enumerating_source(RangeSource &&input)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> enumerating_source<typename std::decay<RangeSource>::type>
#endif
	{
		return enumerating_source<typename std::decay<RangeSource>::type>(std::forward<RangeSource>(input));
	}
}

#endif
