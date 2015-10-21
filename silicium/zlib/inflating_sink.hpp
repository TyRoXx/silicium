#ifndef SILICIUM_ZLIB_INFLATING_SINK_HPP
#define SILICIUM_ZLIB_INFLATING_SINK_HPP

#include <silicium/sink/sink.hpp>
#include <silicium/zlib/zlib.hpp>
#include <silicium/iterator_range.hpp>
#include <silicium/optional.hpp>

#if !SILICIUM_AVOID_ZLIB
namespace Si
{
	struct zlib_inflate_stream
	{
		zlib_inflate_stream() BOOST_NOEXCEPT
		{
		}

		~zlib_inflate_stream() BOOST_NOEXCEPT
		{
			if (!m_stream)
			{
				return;
			}
			inflateEnd(&*m_stream);
		}

		zlib_inflate_stream(zlib_inflate_stream &&other) BOOST_NOEXCEPT
		{
			using std::swap;
			swap(m_stream, other.m_stream);
		}

		zlib_inflate_stream &operator=(zlib_inflate_stream &&other) BOOST_NOEXCEPT
		{
			using std::swap;
			swap(m_stream, other.m_stream);
			return *this;
		}

		std::pair<std::size_t, std::size_t> inflate(iterator_range<char const *> deflated,
		                                            iterator_range<char *> original, int flush) BOOST_NOEXCEPT
		{
			assert(m_stream);
			assert(original.begin());
			assert(original.size());
			assert(deflated.begin());
			assert(deflated.size());
			m_stream->next_in = reinterpret_cast<Bytef *>(const_cast<char *>(deflated.begin()));
			m_stream->avail_in = static_cast<uInt>(deflated.size());
			m_stream->next_out = reinterpret_cast<Bytef *>(original.begin());
			m_stream->avail_out = static_cast<uInt>(original.size());
			int rc = ::inflate(&*m_stream, flush);
			assert(rc != Z_STREAM_ERROR);
			assert(rc != Z_BUF_ERROR);
			assert(rc == Z_OK);
			boost::ignore_unused_variable_warning(rc);
			return std::make_pair(m_stream->avail_in, m_stream->avail_out);
		}

		static zlib_inflate_stream initialize()
		{
			z_stream stream;
			std::memset(&stream, 0, sizeof(stream));
			handle_zlib_status(inflateInit(&stream));
			return zlib_inflate_stream(stream);
		}

	private:
		optional<z_stream> m_stream;

		explicit zlib_inflate_stream(z_stream stream) BOOST_NOEXCEPT : m_stream(stream)
		{
		}

		SILICIUM_DELETED_FUNCTION(zlib_inflate_stream(zlib_inflate_stream const &))
		SILICIUM_DELETED_FUNCTION(zlib_inflate_stream &operator=(zlib_inflate_stream const &))
	};
}
#endif

#endif
