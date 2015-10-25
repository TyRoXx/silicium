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

	namespace detail
	{
		template <class T>
		struct value_wrapper
		{
			value_wrapper()
			    : m_value()
			{
			}

			explicit value_wrapper(T value)
			    : m_value(value)
			{
			}

			template <T Value>
			static value_wrapper create()
			{
				return value_wrapper(Value);
			}

			T value() const
			{
				return m_value;
			}

		private:
			T m_value;
		};
	}

	template <class T, class Length = bounded_size_t, class Data = detail::value_wrapper<T *>>
	struct array_view : private Length, private Data
	{
		typedef T value_type;
		typedef typename std::remove_const<T>::type mutable_value_type;

		array_view()
		    : Length(Length::template literal<0>())
		    , Data(nullptr)
		{
		}

		array_view(T &begin, Length length)
		    : Length(length)
		    , Data(&begin)
		{
		}

		template <std::size_t N>
		array_view(std::array<mutable_value_type, N> &array)
		    : Length(Length::template literal<N>())
		    , Data(array.data())
		{
		}

		template <std::size_t N>
		array_view(std::array<mutable_value_type, N> const &array)
		    : Length(Length::template literal<N>())
		    , Data(array.data())
		{
		}

		template <std::size_t N>
		array_view(boost::array<mutable_value_type, N> &array)
		    : Length(Length::template literal<N>())
		    , Data(array.data())
		{
		}

		template <std::size_t N>
		array_view(boost::array<mutable_value_type, N> const &array)
		    : Length(Length::template literal<N>())
		    , Data(array.data())
		{
		}

		template <class Allocator>
		array_view(std::vector<mutable_value_type, Allocator> &vector)
		    : Length(*Length::create(vector.size()))
		    , Data(vector.data())
		{
			BOOST_STATIC_ASSERT(std::is_same<bounded_size_t, Length>::value);
		}

		template <class Allocator>
		array_view(std::vector<mutable_value_type, Allocator> const &vector)
		    : Length(*Length::create(vector.size()))
		    , Data(vector.data())
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
			return make_iterator_range(begin(), end());
		}

		T *data() const
		{
			return Data::value();
		}

		T *begin() const
		{
			return data();
		}

		T *end() const
		{
			return data() + length().value();
		}
	};
}

#endif
