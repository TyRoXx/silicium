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
		typedef Length length_type;
		typedef typename std::remove_const<T>::type mutable_value_type;
		typedef T *iterator;
		typedef T *const_iterator;
		typedef typename Length::value_type size_type;

		array_view()
		    : Length(Length::template literal<0>())
		    , m_data()
		{
		}

		array_view(value_type &begin, Length length)
		    : Length(length)
		    , m_data(&begin)
		{
		}

		template <std::size_t N>
		array_view(std::array<mutable_value_type, N> &array)
		    : Length(Length::template literal<N>())
		    , m_data(array.data())
		{
		}

		template <std::size_t N>
		array_view(std::array<mutable_value_type, N> const &array)
		    : Length(Length::template literal<N>())
		    , m_data(array.data())
		{
		}

		template <std::size_t N>
		array_view(boost::array<mutable_value_type, N> &array)
		    : Length(Length::template literal<N>())
		    , m_data(array.data())
		{
		}

		template <std::size_t N>
		array_view(boost::array<mutable_value_type, N> const &array)
		    : Length(Length::template literal<N>())
		    , m_data(array.data())
		{
		}

		template <class Allocator>
		array_view(std::vector<mutable_value_type, Allocator> &vector)
		    : Length(*Length::create(vector.size()))
		    , m_data(vector.data())
		{
			BOOST_STATIC_ASSERT(std::is_same<bounded_size_t, Length>::value);
		}

		template <class Allocator>
		array_view(std::vector<mutable_value_type, Allocator> const &vector)
		    : Length(*Length::create(vector.size()))
		    , m_data(vector.data())
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

		iterator_range<value_type *> to_range() const
		{
			return make_iterator_range(begin(), end());
		}

		value_type *data() const
		{
			return m_data;
		}

		iterator begin() const
		{
			return data();
		}

		iterator end() const
		{
			return data() + length().value();
		}

		template <std::size_t MinIndex, std::size_t MaxIndex>
		value_type &operator[](bounded_int<std::size_t, MinIndex, MaxIndex> index) const
		{
			BOOST_STATIC_ASSERT((is_always_less<decltype(index), Length>::value));
			return data()[index.value()];
		}

	private:
		value_type *m_data;
	};

	template <class T, std::size_t N>
	array_view<T, bounded_int<std::size_t, N, N>> make_array_view(std::array<T, N> &array)
	{
		return array_view<T, bounded_int<std::size_t, N, N>>(array);
	}

	template <class T, std::size_t N>
	array_view<T const, bounded_int<std::size_t, N, N>> make_array_view(std::array<T, N> const &array)
	{
		return array_view<T const, bounded_int<std::size_t, N, N>>(array);
	}

	template <class T, std::size_t N>
	array_view<T, bounded_int<std::size_t, N, N>> make_array_view(boost::array<T, N> &array)
	{
		return array_view<T, bounded_int<std::size_t, N, N>>(array);
	}

	template <class T, std::size_t N>
	array_view<T const, bounded_int<std::size_t, N, N>> make_array_view(boost::array<T, N> const &array)
	{
		return array_view<T const, bounded_int<std::size_t, N, N>>(array);
	}

	template <class T, class Allocator>
	array_view<T, bounded_size_t> make_array_view(std::vector<T, Allocator> &vector)
	{
		return array_view<T, bounded_size_t>(vector);
	}

	template <class T, class Allocator>
	array_view<T const, bounded_size_t> make_array_view(std::vector<T, Allocator> const &vector)
	{
		return array_view<T const, bounded_size_t>(vector);
	}
}

#endif
