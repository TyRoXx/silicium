#ifndef SILICIUM_ITERATOR_RANGE_HPP
#define SILICIUM_ITERATOR_RANGE_HPP

#include <silicium/config.hpp>
#include <boost/static_assert.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <cassert>

namespace Si
{
	namespace detail
	{
		//std::iterator_traits<T const *>::value_type seems to be T (GCC 4.8), but that
		//is not useful at all because the const is missing.
		template <class Iterator>
		struct actual_value_type
		{
			typedef typename std::remove_reference<decltype(*std::declval<Iterator>())>::type type;
		};
	}

	template <class Iterator>
	struct iterator_range
	{
		typedef typename std::iterator_traits<Iterator>::difference_type difference_type;
		typedef typename detail::actual_value_type<Iterator>::type value_type;
		typedef Iterator iterator;
		typedef Iterator const_iterator;

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

		template <class OtherIterator>
		iterator_range(iterator_range<OtherIterator> const &other) BOOST_NOEXCEPT
			: m_begin(other.begin())
			, m_end(other.end())
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

		SILICIUM_CXX14_CONSTEXPR void pop_front() BOOST_NOEXCEPT
		{
			assert(!empty());
			pop_front(1);
		}

		value_type &front() const BOOST_NOEXCEPT
		{
			assert(!empty());
			return (*this)[0];
		}

		value_type &operator[](difference_type index) const BOOST_NOEXCEPT
		{
			assert(index < size());
			return begin()[index];
		}

	private:

		Iterator m_begin, m_end;
	};

	template <class Iterator>
	BOOST_CONSTEXPR Iterator const &begin(iterator_range<Iterator> const &range)
	{
		return range.begin();
	}

	//for Boost.Range compatibility
	template <class Iterator>
	BOOST_CONSTEXPR Iterator const &range_begin(iterator_range<Iterator> const &range)
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
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> iterator_range<typename std::decay<Iterator1>::type>
#endif
	{
		typedef typename std::decay<Iterator1>::type iterator_type;
		BOOST_STATIC_ASSERT(std::is_same<iterator_type, typename std::decay<Iterator2>::type>::value);
		return iterator_range<iterator_type>(std::forward<Iterator1>(begin), std::forward<Iterator2>(end));
	}

	template <class Range>
	auto make_iterator_range(Range &&range)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> decltype(make_iterator_range(std::begin(range), std::end(range)))
#endif
	{
		using std::begin;
		using std::end;
		return make_iterator_range(begin(range), end(range));
	}

	template <class ContiguousRange>
	auto make_contiguous_range(ContiguousRange &&range)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> decltype(make_iterator_range(&*std::begin(range), &*std::end(range)))
#endif
	{
		using std::begin;
		using std::end;
		auto * const data_begin = &*begin(range);
		auto * const data_end = &*end(range);
		return make_iterator_range(data_begin, data_end);
	}
}

#endif
