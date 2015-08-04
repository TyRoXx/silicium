#ifndef SILICIUM_SERIALIZATION_HPP
#define SILICIUM_SERIALIZATION_HPP

#include <silicium/config.hpp>
#include <silicium/sink/sink.hpp>
#include <silicium/source/source.hpp>
#include <silicium/optional.hpp>

namespace Si
{
	namespace serialization
	{
		typedef boost::uint8_t byte;

		struct big_endian
		{
			static BOOST_CONSTEXPR std::size_t byte_index_to_significance(std::size_t index, std::size_t total_size)
			{
				return (total_size - 1 - index);
			}
		};

		struct little_endian
		{
			static BOOST_CONSTEXPR std::size_t byte_index_to_significance(std::size_t index, std::size_t total_size)
			{
				boost::ignore_unused_variable_warning(total_size);
				return index;
			}
		};

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
				built |= (digit << (significance * 8U));
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
				while ((begin != end) && (m_bytes_in_buffer < sizeof(m_buffer)))
				{
					m_buffer.set_byte(Endianness::byte_index_to_significance(m_bytes_in_buffer, sizeof(m_buffer)), *begin);
					++begin;
					++m_bytes_in_buffer;
				}
				return begin;
			}

			BOOST_CONSTEXPR Unsigned const *check_result() const BOOST_NOEXCEPT
			{
				if (m_bytes_in_buffer == sizeof(m_buffer))
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
