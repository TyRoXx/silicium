#include <silicium/config.hpp>
#include <silicium/bounded_int.hpp>
#include <silicium/variant.hpp>
#include <silicium/arithmetic/add.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	namespace m2
	{
		using std::size_t;
		using std::move;

		struct empty_t
		{
		};

		static BOOST_CONSTEXPR_OR_CONST empty_t empty;

		struct out_of_memory
		{
		};

		struct piece_of_memory
		{
			void *begin;

			piece_of_memory()
			    : begin(nullptr)
			{
			}

			explicit piece_of_memory(void *begin)
			    : begin(begin)
			{
			}
		};

		struct standard_allocator
		{
			variant<piece_of_memory, out_of_memory> allocate(size_t size)
			{
				void *memory = std::malloc(size);
				if (memory)
				{
					return piece_of_memory{memory};
				}
				return out_of_memory();
			}

			variant<piece_of_memory, out_of_memory> reallocate(piece_of_memory existing_allocation, size_t new_size)
			{
				void *new_allocation = std::realloc(existing_allocation.begin, new_size);
				if (new_allocation)
				{
					return piece_of_memory{new_allocation};
				}
				return out_of_memory();
			}

			void deallocate(piece_of_memory existing_allocation)
			{
				std::free(existing_allocation.begin);
			}
		};

		enum class resize_result
		{
			success,
			out_of_memory
		};

		template <class T, class Length, class Allocator>
		struct dynamic_storage : private Allocator
		{
			typedef T element_type;
			typedef Length length_type;

			dynamic_storage(empty_t, Allocator allocator = Allocator())
			    : Allocator(move(allocator))
			    , m_begin()
			    , m_length(length_type::template literal<0>())
			{
			}

			~dynamic_storage()
			{
				Allocator::deallocate(m_begin);
			}

			length_type length() const
			{
				return m_length;
			}

			resize_result resize(length_type new_length)
			{
				// TODO: handle overflow
				size_t const size_in_bytes = new_length.value() * sizeof(element_type);
				return visit<resize_result>(Allocator::reallocate(m_begin, size_in_bytes),
				                            [this, new_length](piece_of_memory const reallocated)
				                            {
					                            m_begin = reallocated;
					                            m_length = new_length;
					                            return resize_result::success;
					                        },
				                            [](out_of_memory)
				                            {
					                            return resize_result::out_of_memory;
					                        });
			}

			T &data() const
			{
				return *begin();
			}

			dynamic_storage(dynamic_storage &&other) BOOST_NOEXCEPT : Allocator(move(other)),
			                                                          m_begin(other.m_begin),
			                                                          m_length(other.m_length)
			{
			}

			dynamic_storage &operator=(dynamic_storage &&other) BOOST_NOEXCEPT
			{
				std::memcpy(this, &other, sizeof(other));
				return *this;
			}

			SILICIUM_DISABLE_COPY(dynamic_storage)

		private:
			piece_of_memory m_begin;
			length_type m_length;

			T *begin() const
			{
				return static_cast<T *>(m_begin.begin);
			}
		};

		enum class emplace_back_result
		{
			success,
			full
		};

		template <class DynamicStorage>
		struct basic_vector
		{
			typedef typename DynamicStorage::element_type element_type;
			typedef typename DynamicStorage::length_type length_type;

			basic_vector(empty_t, DynamicStorage storage)
			    : m_storage(move(storage))
			    , m_used(length_type::template literal<0>())
			{
			}

			length_type capacity() const
			{
				return m_storage.length();
			}

			length_type length() const
			{
				return m_used;
			}

			template <class... Args>
			emplace_back_result emplace_back(Args &&... args)
			{
				overflow_or<typename length_type::value_type> const maybe_new_size =
				    checked_add<typename length_type::value_type>(capacity().value(), 1);
				if (maybe_new_size.is_overflow())
				{
					return emplace_back_result::full;
				}
				length_type const new_size = *length_type::create(*maybe_new_size.value());
				if (m_storage.length() < new_size)
				{
					// TODO: exponential growth
					// TODO: handle out-of-memory situation
					m_storage.resize(new_size);
				}
				new (&m_storage.data()) element_type{std::forward<Args>(args)...};
				m_used = new_size;
				return emplace_back_result::success;
			}

			basic_vector(basic_vector &&other) BOOST_NOEXCEPT : m_storage(other.m_storage), m_used(other.m_used)
			{
			}

			basic_vector &operator=(basic_vector &&other) BOOST_NOEXCEPT
			{
				std::memcpy(this, &other, sizeof(other));
				return *this;
			}

			SILICIUM_DISABLE_COPY(basic_vector)

		private:
			DynamicStorage m_storage;
			length_type m_used;
		};

		template <class T, class Length>
		using vector = basic_vector<dynamic_storage<T, Length, standard_allocator>>;
	}
}

BOOST_AUTO_TEST_CASE(move2_vector_emplace_back)
{
	Si::m2::vector<std::uint64_t, Si::bounded_int<std::size_t, 0, 10>> v{
	    Si::m2::empty,
	    Si::m2::dynamic_storage<std::uint64_t, Si::bounded_int<std::size_t, 0, 10>, Si::m2::standard_allocator>{
	        Si::m2::empty}};
	BOOST_REQUIRE_EQUAL(0u, v.length().value());
	BOOST_REQUIRE_EQUAL(0u, v.capacity().value());
	BOOST_REQUIRE(Si::m2::emplace_back_result::success == v.emplace_back(12u));
	BOOST_REQUIRE_EQUAL(1u, v.length().value());
	BOOST_REQUIRE_EQUAL(1u, v.capacity().value());
	BOOST_REQUIRE(Si::m2::emplace_back_result::success == v.emplace_back(13u));
	BOOST_REQUIRE_EQUAL(2u, v.length().value());
	BOOST_REQUIRE_EQUAL(2u, v.capacity().value());
}
