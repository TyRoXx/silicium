#ifndef SILICIUM_ITERATOR_RANGE_HPP
#define SILICIUM_ITERATOR_RANGE_HPP

#include <silicium/config.hpp>
#include <silicium/is_handle.hpp>
#include <boost/static_assert.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/utility/addressof.hpp>
#include <cassert>

namespace Si
{
	namespace detail
	{
		// std::iterator_traits<T const *>::value_type seems to be T (GCC 4.8), but that
		// is not useful at all because the const is missing.
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

		BOOST_CONSTEXPR iterator_range() BOOST_NOEXCEPT : m_begin(Iterator()), m_end(Iterator())
		{
		}

		template <class It1, class It2>
		BOOST_CONSTEXPR iterator_range(It1 &&begin, It2 &&end) BOOST_NOEXCEPT : m_begin(std::forward<It1>(begin)),
		                                                                        m_end(std::forward<It2>(end))
		{
		}

		template <class OtherIterator>
		iterator_range(iterator_range<OtherIterator> const &other) BOOST_NOEXCEPT : m_begin(other.begin()),
		                                                                            m_end(other.end())
		{
		}

		template <class OtherIterator>
		iterator_range &operator=(iterator_range<OtherIterator> const &other) BOOST_NOEXCEPT
		{
			m_begin = other.begin();
			m_end = other.end();
			return *this;
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

		/// This method is only available with random access iterators. Use std::distance(begin(r), end(r))
		/// if you really want a potential O(n) operation.
		BOOST_CONSTEXPR difference_type size() const BOOST_NOEXCEPT
		{
			return m_end - m_begin;
		}

		/// This method is only available with random access iterators.
		/// If you need this for weaker iterators, use a loop that calls pop_front() for individual elements.
		SILICIUM_CXX14_CONSTEXPR void pop_front(difference_type n) BOOST_NOEXCEPT
		{
			assert(n <= size());
			m_begin += n;
		}

		SILICIUM_CXX14_CONSTEXPR void pop_front() BOOST_NOEXCEPT
		{
			assert(!empty());
			++m_begin;
		}

		value_type &front() const BOOST_NOEXCEPT
		{
			assert(!empty());
			return *begin();
		}

		/// This method is only available with random access iterators so that it takes O(1) in time.
		value_type &operator[](difference_type index) const BOOST_NOEXCEPT
		{
			(void)static_cast<std::random_access_iterator_tag>(
			    typename std::iterator_traits<Iterator>::iterator_category{});
			assert(index < size());
			return begin()[index];
		}

	private:
		Iterator m_begin, m_end;
	};

	template <class Iterator1, class Iterator2>
	BOOST_CONSTEXPR bool pointing_to_same_subrange(iterator_range<Iterator1> const &left,
	                                               iterator_range<Iterator2> const &right)
	{
		return (left.begin() == right.begin()) && (left.end() == right.end());
	}

	template <class Iterator>
	BOOST_CONSTEXPR Iterator const &begin(iterator_range<Iterator> const &range)
	{
		return range.begin();
	}

	// for Boost.Range compatibility
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
		BOOST_STATIC_ASSERT((std::is_same<iterator_type, typename std::decay<Iterator2>::type>::value));
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
	    -> decltype(make_iterator_range(boost::addressof(*std::begin(range)), boost::addressof(*std::end(range))))
#endif
	{
		using std::begin;
		using std::end;
		auto begin_ = begin(range);
		auto end_ = end(range);
		if (begin_ == end_)
		{
			typename std::remove_reference<decltype(*begin_)>::type *data = nullptr;
			return make_iterator_range(data, data);
		}
		auto *const data_begin = boost::addressof(*begin_);
		auto *const data_end = data_begin + std::distance(begin_, end_);
		return make_iterator_range(data_begin, data_end);
	}
}

#endif
