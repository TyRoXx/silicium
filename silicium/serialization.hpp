#ifndef SILICIUM_SERIALIZATION_HPP
#define SILICIUM_SERIALIZATION_HPP

#include <silicium/config.hpp>
#include <silicium/optional.hpp>
#include <silicium/byte_order_intrinsics.hpp>
#include <silicium/identity.hpp>
#include <boost/concept_check.hpp>
#include <cstring>

namespace Si
{
	namespace serialization
	{
		typedef boost::uint8_t byte;

		struct big_endian
		{
			static BOOST_CONSTEXPR std::size_t
			byte_index_to_significance(std::size_t index,
			                           std::size_t total_size)
			{
				return (total_size - 1 - index);
			}
		};

		struct little_endian
		{
			static BOOST_CONSTEXPR std::size_t
			byte_index_to_significance(std::size_t index, std::size_t)
			{
				return index;
			}
		};

		template <class Unsigned, class Endianness, class InputIterator>
		optional<std::pair<Unsigned, InputIterator>>
		    bytes_to_integer_shortcut(InputIterator, InputIterator, Endianness,
		                              identity<Unsigned>) BOOST_NOEXCEPT
		{
			return none;
		}

		inline optional<std::pair<boost::uint32_t, byte const *>>
		bytes_to_integer_shortcut(byte const *begin, byte const *end,
		                          big_endian,
		                          identity<boost::uint32_t>) BOOST_NOEXCEPT
		{
			if (static_cast<size_t>(end - begin) < sizeof(boost::uint32_t))
			{
				return none;
			}
			boost::uint32_t result;
			std::memcpy(&result, begin, sizeof(result));
			result = ntoh32(result);
			return std::make_pair(result, begin + sizeof(result));
		}

		template <class Unsigned>
		struct unsigned_int_builder
		{
			Unsigned built;

			unsigned_int_builder()
			    : built()
			{
			}

			void set_byte(std::size_t significance, byte digit)
			{
				built |= (static_cast<Unsigned>(digit)
				          << static_cast<Unsigned>(significance * 8U));
			}
		};

		template <class Unsigned, class Endianness>
		struct endian_parser
		{
			endian_parser()
			    : m_bytes_in_buffer(0)
			{
			}

			template <class InputIterator>
			InputIterator consume_input(InputIterator begin, InputIterator end)
			{
				if (m_bytes_in_buffer == 0)
				{
					optional<std::pair<Unsigned, InputIterator>> const
					    succeeded = bytes_to_integer_shortcut(
					        begin, end, Endianness(), identity<Unsigned>());
					if (succeeded)
					{
						m_buffer.built = succeeded->first;
						m_bytes_in_buffer = sizeof(Unsigned);
						return succeeded->second;
					}
				}
				while ((begin != end) && (m_bytes_in_buffer < sizeof(Unsigned)))
				{
					m_buffer.set_byte(Endianness::byte_index_to_significance(
					                      m_bytes_in_buffer, sizeof(Unsigned)),
					                  *begin);
					++begin;
					++m_bytes_in_buffer;
				}
				return begin;
			}

			BOOST_CONSTEXPR Unsigned const *check_result() const BOOST_NOEXCEPT
			{
				if (m_bytes_in_buffer == sizeof(Unsigned))
				{
					return &m_buffer.built;
				}
				return nullptr;
			}

		private:
			unsigned_int_builder<Unsigned> m_buffer;
			std::size_t m_bytes_in_buffer;
		};

		typedef endian_parser<boost::uint8_t, big_endian> be_uint8_parser;
		typedef endian_parser<boost::uint16_t, big_endian> be_uint16_parser;
		typedef endian_parser<boost::uint32_t, big_endian> be_uint32_parser;
		typedef endian_parser<boost::uint64_t, big_endian> be_uint64_parser;

		typedef endian_parser<boost::uint8_t, little_endian> le_uint8_parser;
		typedef endian_parser<boost::uint16_t, little_endian> le_uint16_parser;
		typedef endian_parser<boost::uint32_t, little_endian> le_uint32_parser;
		typedef endian_parser<boost::uint64_t, little_endian> le_uint64_parser;
	}
}

#endif
