#include <silicium/config.hpp>
#include <silicium/bounded_int.hpp>
#include <silicium/variant.hpp>
#include <silicium/array_view.hpp>
#include <silicium/arithmetic/add.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	namespace m3
	{
		template <class T>
		void trivial_copy(T &destination, T const &source)
		{
			std::memcpy(&destination, &source, sizeof(source));
		}

		template <class T>
		struct val
		{
			val() BOOST_NOEXCEPT : m_is_set(false)
			{
			}

			template <class... Args>
			explicit val(Args &&... args)
			    : m_is_set(true)
			{
				new (static_cast<void *>(&get())) T(std::forward<Args>(args)...);
			}

			~val() BOOST_NOEXCEPT
			{
				if (!m_is_set)
				{
					return;
				}
				get().~T();
			}

			val(val &&other) BOOST_NOEXCEPT : m_is_set(other.m_is_set)
			{
				if (m_is_set)
				{
					trivial_copy(get(), other.get());
					other.m_is_set = false;
				}
			}

			val &operator=(val &&other) BOOST_NOEXCEPT
			{
				if (m_is_set)
				{
					if (other.m_is_set)
					{
						get().~T();
						trivial_copy(get(), other.get());
						other.m_is_set = false;
					}
					else
					{
						get().~T();
						m_is_set = false;
					}
				}
				else
				{
					if (other.m_is_set)
					{
						trivial_copy(get(), other.get());
						other.m_is_set = false;
						m_is_set = true;
					}
					else
					{
						// nothing to do
					}
				}
				return *this;
			}

			void release() BOOST_NOEXCEPT
			{
				m_is_set = false;
			}

			T &require()
			{
				if (!m_is_set)
				{
					boost::throw_exception(std::logic_error("expected non-empty val"));
				}
				return get();
			}

			void transfer(T &destination) BOOST_NOEXCEPT
			{
				trivial_copy(destination, require());
				release();
			}

			static val steal(T const &from)
			{
				val result;
				trivial_copy(result.get(), from);
				result.m_is_set = true;
				return result;
			}

		private:
			bool m_is_set;
			typename std::aligned_storage<sizeof(T), alignof(T)>::type m_storage;

			T &get()
			{
				return reinterpret_cast<T &>(reinterpret_cast<char &>(m_storage));
			}
		};

		template <class T>
		val<T> steal(T const &stolen)
		{
			return val<T>::steal(stolen);
		}

		template <class T, class Deleter>
		struct unique_ref : private Deleter
		{
			explicit unique_ref(T &ref, Deleter deleter) BOOST_NOEXCEPT : Deleter(deleter), m_ptr(&ref)
			{
			}

			explicit unique_ref(val<unique_ref> other) BOOST_NOEXCEPT
			{
				other.transfer(*this);
			}

			~unique_ref() BOOST_NOEXCEPT
			{
				Deleter::operator()(*m_ptr);
			}

			SILICIUM_DISABLE_COPY(unique_ref)

			T &ref() const BOOST_NOEXCEPT
			{
				return *m_ptr;
			}

		private:
			T *m_ptr;
		};

		struct new_deleter
		{
			new_deleter() BOOST_NOEXCEPT
			{
			}

			template <class T>
			void operator()(T &deleted) const BOOST_NOEXCEPT
			{
				delete &deleted;
			}
		};

		struct malloc_deleter
		{
			malloc_deleter() BOOST_NOEXCEPT
			{
			}

			template <class T>
			void operator()(T &deleted) const BOOST_NOEXCEPT
			{
				std::free(&deleted);
			}
		};

		template <class T, class... Args>
		val<unique_ref<T, new_deleter>> make_unique(Args &&... args)
		{
			return val<unique_ref<T, new_deleter>>(*new T(std::forward<Args>(args)...), new_deleter());
		}

		template <class T>
		val<unique_ref<T, malloc_deleter>> allocate_array_storage(std::size_t length)
		{
			void *const storage = std::calloc(length, sizeof(T));
			if (!storage)
			{
				boost::throw_exception(std::bad_alloc());
			}
			return val<unique_ref<T, malloc_deleter>>(*static_cast<T *>(storage), malloc_deleter());
		}

		template <class T, class Length>
		struct dynamic_array : private Length
		{
			template <class ElementGenerator>
			dynamic_array(Length length, ElementGenerator &&generate_elements) BOOST_NOEXCEPT
			    : Length(length),
			      m_elements(allocate_array_storage<T>(length.value()))
			{
				for (typename Length::value_type i = 0, c = this->length().value(); i < c; ++i)
				{
					generate_elements().transfer((&m_elements.ref())[i]);
				}
			}

			explicit dynamic_array(val<dynamic_array> other) BOOST_NOEXCEPT
			    : Length(other.require()),
			      m_elements(steal(other.require().m_elements))
			{
				other.release();
			}

			~dynamic_array() BOOST_NOEXCEPT
			{
				for (typename Length::value_type i = 0, c = length().value(); i < c; ++i)
				{
					(&m_elements.ref())[c - i - 1].~T();
				}
			}

			Length length() const BOOST_NOEXCEPT
			{
				return *this;
			}

			array_view<T, Length> as_view() const BOOST_NOEXCEPT
			{
				return array_view<T, Length>(m_elements.ref(), length());
			}

			SILICIUM_DISABLE_COPY(dynamic_array)

		private:
			unique_ref<T, malloc_deleter> m_elements;
		};

#define SILICIUM_M3_HAS_TUPLE SILICIUM_COMPILER_HAS_VARIADIC_PACK_EXPANSION

#if SILICIUM_M3_HAS_TUPLE
		template <std::size_t I, class... T>
		struct type_at;

		template <class Head, class... Tail>
		struct type_at<0, Head, Tail...>
		{
			typedef Head type;
		};

		template <std::size_t I, class Head, class... Tail>
		struct type_at<I, Head, Tail...>
		{
			typedef typename type_at<I - 1, Tail...>::type type;
		};

		template <class... T>
		struct tuple_impl;

		template <>
		struct tuple_impl<>
		{
			tuple_impl() BOOST_NOEXCEPT
			{
			}

			explicit tuple_impl(tuple_impl const &) BOOST_NOEXCEPT
			{
			}
		};

		template <class Head, class... Tail>
		struct tuple_impl<Head, Tail...> : private tuple_impl<Tail...>
		{
			template <class First, class... Rest>
			explicit tuple_impl(First &&first, Rest &&... rest) BOOST_NOEXCEPT : base(std::forward<Rest>(rest)...),
			                                                                     m_head(std::forward<First>(first))
			{
			}

			explicit tuple_impl(tuple_impl<Head, Tail...> const &stolen) BOOST_NOEXCEPT
			    : base(static_cast<base const &>(stolen)),
			      m_head(steal(stolen.m_head))
			{
			}

			Head &get(std::integral_constant<std::size_t, 0>)
			{
				return m_head;
			}

			template <std::size_t I>
			typename type_at<I - 1, Tail...>::type &get(std::integral_constant<std::size_t, I>)
			{
				return base::template get(std::integral_constant<std::size_t, I - 1>());
			}

		private:
			typedef tuple_impl<Tail...> base;

			Head m_head;
		};

		template <class... T>
		struct tuple : private tuple_impl<T...>
		{
			template <class... Elements>
			explicit tuple(Elements &&... elements) BOOST_NOEXCEPT : base(std::forward<Elements>(elements)...)
			{
			}

			explicit tuple(val<tuple> other) BOOST_NOEXCEPT : base(static_cast<base const &>(other.require()))
			{
				other.release();
			}

			template <std::size_t I>
			typename type_at<I, T...>::type &get()
			{
				return base::template get(std::integral_constant<std::size_t, I>());
			}

		private:
			typedef tuple_impl<T...> base;
		};

		template <std::size_t I, class... T>
		typename type_at<I, T...>::type &get(tuple<T...> &from)
		{
			return from.template get<I>();
		}

		template <class T>
		struct make_tuple_decay
		{
			typedef T type;
		};

		template <class T>
		struct make_tuple_decay<val<T>>
		{
			typedef T type;
		};

		template <class... T>
		auto make_tuple(T &&... elements)
		    -> val<tuple<typename make_tuple_decay<typename std::decay<T>::type>::type>...>
		{
			return val<tuple<typename make_tuple_decay<typename std::decay<T>::type>::type>...>(
			    std::forward<T>(elements)...);
		}
#endif
	}
}

using namespace Si::m3;

BOOST_AUTO_TEST_CASE(move3_val)
{
	val<unique_ref<int, new_deleter>> r = make_unique<int>(23);
	BOOST_CHECK_EQUAL(23, r.require().ref());
	val<unique_ref<int, new_deleter>> s = std::move(r);
	BOOST_CHECK_EQUAL(23, s.require().ref());
	s.require().ref() = 24;
	r = std::move(s);
	BOOST_CHECK_EQUAL(24, r.require().ref());
	unique_ref<int, new_deleter> t(std::move(r));
	BOOST_CHECK_EQUAL(24, t.ref());
}

BOOST_AUTO_TEST_CASE(move3_vector_ref_emplace_back)
{
	typedef Si::bounded_int<std::size_t, 1, 2> length_type;
	dynamic_array<unique_ref<int, new_deleter>, length_type> const v(length_type::literal<1>(), []()
	                                                                 {
		                                                                 return make_unique<int>(23);
		                                                             });
	BOOST_REQUIRE_EQUAL(length_type::literal<1>(), v.length());
	Si::array_view<unique_ref<int, new_deleter>, length_type> const range = v.as_view();
	BOOST_REQUIRE_EQUAL(length_type::literal<1>(), range.length());
	unique_ref<int, new_deleter> const &element = range[Si::literal<std::size_t, 0>()];
	BOOST_CHECK_EQUAL(element.ref(), 23);
}

BOOST_AUTO_TEST_CASE(move3_val_of_vector)
{
	typedef Si::bounded_int<std::size_t, 1, 2> length_type;
	auto create_array = []()
	{
		return val<dynamic_array<unique_ref<int, new_deleter>, length_type>>(length_type::literal<1>(), []()
		                                                                     {
			                                                                     return make_unique<int>(23);
			                                                                 });
	};
	val<dynamic_array<unique_ref<int, new_deleter>, length_type>> a = create_array();
	dynamic_array<unique_ref<int, new_deleter>, length_type> const v(std::move(a));
	BOOST_REQUIRE_EQUAL(length_type::literal<1>(), v.length());
	Si::array_view<unique_ref<int, new_deleter>, length_type> const range = v.as_view();
	BOOST_REQUIRE_EQUAL(length_type::literal<1>(), range.length());
	unique_ref<int, new_deleter> const &element = range[Si::literal<std::size_t, 0>()];
	BOOST_CHECK_EQUAL(element.ref(), 23);
}

#if SILICIUM_M3_HAS_TUPLE
BOOST_AUTO_TEST_CASE(move3_make_tuple)
{
	val<tuple<unique_ref<int, new_deleter>>> t = make_tuple(make_unique<int>(23));
	tuple<unique_ref<int, new_deleter>> u(std::move(t));
	unique_ref<int, new_deleter> &element = get<0>(u);
	BOOST_CHECK_EQUAL(23, element.ref());
}
#endif
