#ifndef SILICIUM_ITERATOR_RANGE_HPP
#define SILICIUM_ITERATOR_RANGE_HPP

#include <silicium/config.hpp>

namespace Si
{
	template <class Iterator>
	struct iterator_range
	{
		typedef typename std::iterator_traits<Iterator>::difference_type difference_type;
		typedef typename std::iterator_traits<Iterator>::value_type value_type;

		BOOST_CONSTEXPR iterator_range() BOOST_NOEXCEPT
			: m_begin(Iterator())
			, m_end(Iterator())
		{
		}

		template <class It1, class It2>
		BOOST_CONSTEXPR iterator_range(It1 &&begin, It2 &&end) BOOST_NOEXCEPT
			: m_begin(std::forward<It1>(begin))
			, m_end(std::forward<It2>(end))
		{
		}

		BOOST_CONSTEXPR Iterator const &begin() const BOOST_NOEXCEPT
		{
			return m_begin;
		}

		BOOST_CONSTEXPR Iterator const &end() const BOOST_NOEXCEPT
		{
			return m_end;
		}

		BOOST_CONSTEXPR bool empty() const BOOST_NOEXCEPT
		{
			return m_begin == m_end;
		}

		BOOST_CONSTEXPR difference_type size() const BOOST_NOEXCEPT
		{
			return std::distance(m_begin, m_end);
		}

		SILICIUM_CXX14_CONSTEXPR void pop_front(difference_type n) BOOST_NOEXCEPT
		{
			assert(n <= size());
			std::advance(m_begin, n);
		}

	private:

		Iterator m_begin, m_end;
	};

	template <class Iterator>
	BOOST_CONSTEXPR Iterator const &begin(iterator_range<Iterator> const &range)
	{
		return range.begin();
	}

	template <class Iterator>
	BOOST_CONSTEXPR Iterator const &end(iterator_range<Iterator> const &range)
	{
		return range.end();
	}

	template <class Iterator1, class Iterator2>
	BOOST_CONSTEXPR auto make_iterator_range(Iterator1 &&begin, Iterator2 &&end)
	{
		typedef typename std::decay<Iterator1>::type iterator_type;
		BOOST_STATIC_ASSERT(std::is_same<iterator_type, typename std::decay<Iterator2>::type>::value);
		return iterator_range<iterator_type>(std::forward<Iterator1>(begin), std::forward<Iterator2>(end));
	}
}

#endif
