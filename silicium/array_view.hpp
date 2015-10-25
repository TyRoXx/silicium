#ifndef SILICIUM_ARRAY_VIEW_HPP
#define SILICIUM_ARRAY_VIEW_HPP

#include <silicium/bounded_int.hpp>
#include <silicium/iterator_range.hpp>
#include <array>
#include <boost/array.hpp>

namespace Si
{
	typedef Si::bounded_int<std::size_t, 0,
#if SILICIUM_COMPILER_HAS_CONSTEXPR_NUMERIC_LIMITS
	                        (std::numeric_limits<std::size_t>::max)()
#else
	                        SIZE_MAX
#endif
	                        > bounded_size_t;

	template <class T, class Length = bounded_size_t>
	struct array_view : private Length
	{
		typedef T value_type;
		typedef typename std::remove_const<T>::type mutable_value_type;

		array_view()
		    : Length(Length::template literal<0>())
		    , m_begin(nullptr)
		{
		}

		array_view(T &begin, Length length)
		    : Length(length)
		    , m_begin(&begin)
		{
		}

		template <std::size_t N>
		array_view(std::array<mutable_value_type, N> &array)
		    : Length(Length::template literal<N>())
		    , m_begin(array.data())
		{
		}

		template <std::size_t N>
		array_view(std::array<mutable_value_type, N> const &array)
		    : Length(Length::template literal<N>())
		    , m_begin(array.data())
		{
		}

		template <std::size_t N>
		array_view(boost::array<mutable_value_type, N> &array)
		    : Length(Length::template literal<N>())
		    , m_begin(array.data())
		{
		}

		template <std::size_t N>
		array_view(boost::array<mutable_value_type, N> const &array)
		    : Length(Length::template literal<N>())
		    , m_begin(array.data())
		{
		}

		template <class Allocator>
		array_view(std::vector<mutable_value_type, Allocator> &vector)
		    : Length(*Length::create(vector.size()))
		    , m_begin(vector.data())
		{
			BOOST_STATIC_ASSERT(std::is_same<bounded_size_t, Length>::value);
		}

		template <class Allocator>
		array_view(std::vector<mutable_value_type, Allocator> const &vector)
		    : Length(*Length::create(vector.size()))
		    , m_begin(vector.data())
		{
			BOOST_STATIC_ASSERT(std::is_same<bounded_size_t, Length>::value);
		}

		Length length() const
		{
			return *this;
		}

		bool empty() const
		{
			return length().value() == 0;
		}

		iterator_range<T *> to_range() const
		{
			return make_iterator_range(m_begin, m_begin + length().value());
		}

	private:
		T *m_begin;
	};
}

#endif
