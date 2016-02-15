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
		struct absorb
		{
		};

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
					new (static_cast<void *>(&get())) T(absorb(), other.get());
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
						new (static_cast<void *>(&get())) T(absorb(), other.get());
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
						new (static_cast<void *>(&get())) T(absorb(), other.get());
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
			explicit unique_ref(T &ref, Deleter deleter) BOOST_NOEXCEPT : Deleter(deleter), m_ref(ref)
			{
			}

			unique_ref(val<unique_ref> &&other) BOOST_NOEXCEPT : Deleter(other.require()), m_ref(other.require().m_ref)
			{
				other.release();
			}

			unique_ref(absorb, unique_ref const &other) BOOST_NOEXCEPT : Deleter(other), m_ref(other.m_ref)
			{
			}

			~unique_ref() BOOST_NOEXCEPT
			{
				Deleter::operator()(m_ref);
			}

			SILICIUM_DISABLE_COPY(unique_ref)

			T &ref() const BOOST_NOEXCEPT
			{
				return m_ref;
			}

		private:
			T &m_ref;
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
		struct dynamic_array
		{
			template <class ElementGenerator>
			dynamic_array(Length length, ElementGenerator &&generate_elements) BOOST_NOEXCEPT
			    : m_elements(allocate_array_storage<T>(length.value())),
			      m_length(length)
			{
				for (typename Length::value_type i = 0, c = m_length.value(); i < c; ++i)
				{
					new (static_cast<void *>((&m_elements.ref()) + i)) T(generate_elements());
				}
			}

			Length length() const BOOST_NOEXCEPT
			{
				return m_length;
			}

			array_view<T, Length> as_view() const BOOST_NOEXCEPT
			{
				return array_view<T, Length>(m_elements.ref(), length());
			}

		private:
			unique_ref<T, malloc_deleter> m_elements;
			Length m_length;
		};
	}
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
