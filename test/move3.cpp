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

		private:
			bool m_is_set;
			typename std::aligned_storage<sizeof(T), alignof(T)>::type m_storage;

			T &get()
			{
				return reinterpret_cast<T &>(reinterpret_cast<char &>(m_storage));
			}
		};

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
	}
}

BOOST_AUTO_TEST_CASE(move3_val)
{
	Si::m3::val<Si::m3::unique_ref<int, Si::m3::new_deleter>> r = Si::m3::make_unique<int>(23);
	BOOST_CHECK_EQUAL(23, r.require().ref());
	Si::m3::val<Si::m3::unique_ref<int, Si::m3::new_deleter>> s = std::move(r);
	BOOST_CHECK_EQUAL(23, s.require().ref());
	s.require().ref() = 24;
	r = std::move(s);
	BOOST_CHECK_EQUAL(24, r.require().ref());
	Si::m3::unique_ref<int, Si::m3::new_deleter> t(std::move(r));
	BOOST_CHECK_EQUAL(24, t.ref());
}

BOOST_AUTO_TEST_CASE(move3_vector_ref_emplace_back)
{
	typedef Si::bounded_int<std::size_t, 1, 2> length_type;
	Si::m3::dynamic_array<Si::m3::unique_ref<int, Si::m3::new_deleter>, length_type> const v(
	    length_type::literal<1>(), []()
	    {
		    return Si::m3::make_unique<int>(23);
		});
	BOOST_REQUIRE_EQUAL(length_type::literal<1>(), v.length());
	Si::array_view<Si::m3::unique_ref<int, Si::m3::new_deleter>, length_type> const range = v.as_view();
	BOOST_REQUIRE_EQUAL(length_type::literal<1>(), range.length());
	Si::m3::unique_ref<int, Si::m3::new_deleter> const &element = range[Si::literal<std::size_t, 0>()];
	BOOST_CHECK_EQUAL(element.ref(), 23);
}
