#ifndef SILICIUM_ZLIB_DEFLATING_SINK_HPP
#define SILICIUM_ZLIB_DEFLATING_SINK_HPP

#include <silicium/variant.hpp>
#include <silicium/sink/sink.hpp>
#include <silicium/zlib/zlib.hpp>
#include <silicium/optional.hpp>
#include <ventura/flush.hpp>
#include <silicium/iterator_range.hpp>
#include <silicium/memory_range.hpp>

#define SILICIUM_HAS_DEFLATING_SINK (SILICIUM_HAS_VARIANT && !SILICIUM_AVOID_ZLIB)

#if SILICIUM_HAS_DEFLATING_SINK
namespace Si
{
	struct zlib_deflate_stream
	{
		zlib_deflate_stream() BOOST_NOEXCEPT
		{
		}

		explicit zlib_deflate_stream(int level)
		{
			z_stream zero;
			std::memset(&zero, 0, sizeof(zero));
			m_stream = zero;
			handle_zlib_status(deflateInit(&*m_stream, level));
		}

		zlib_deflate_stream(zlib_deflate_stream &&other) BOOST_NOEXCEPT
		{
			using std::swap;
			swap(m_stream, other.m_stream);
		}

		zlib_deflate_stream &operator = (zlib_deflate_stream &&other) BOOST_NOEXCEPT
		{
			using std::swap;
			swap(m_stream, other.m_stream);
			return *this;
		}

		~zlib_deflate_stream() BOOST_NOEXCEPT
		{
			if (!m_stream)
			{
				return;
			}
			deflateEnd(&*m_stream);
		}

		std::pair<std::size_t, std::size_t> deflate(
			iterator_range<char const *> original,
			iterator_range<char *> deflated,
			int flush
			) BOOST_NOEXCEPT
		{
			assert(m_stream);
			assert(deflated.begin());
			assert(deflated.size());
			m_stream->next_in = reinterpret_cast<Bytef *>(const_cast<char *>(original.begin()));
			m_stream->avail_in = static_cast<uInt>(original.size());
			m_stream->next_out = reinterpret_cast<Bytef *>(deflated.begin());
			m_stream->avail_out = static_cast<uInt>(deflated.size());
			int rc = ::deflate(&*m_stream, flush);
			assert(rc != Z_STREAM_ERROR);
			assert(rc != Z_BUF_ERROR);
			assert(rc == Z_OK);
			boost::ignore_unused_variable_warning(rc);
			return std::make_pair(m_stream->avail_in, m_stream->avail_out);
		}

	private:

		optional<z_stream> m_stream;

		SILICIUM_DELETED_FUNCTION(zlib_deflate_stream(zlib_deflate_stream const &))
		SILICIUM_DELETED_FUNCTION(zlib_deflate_stream &operator = (zlib_deflate_stream const &))
	};

	typedef variant<flush, memory_range> zlib_sink_element;

	template <class Next>
	struct zlib_deflating_sink
	{
		typedef zlib_sink_element element_type;
		typedef boost::system::error_code error_type;

		zlib_deflating_sink()
		{
		}

		explicit zlib_deflating_sink(Next next, zlib_deflate_stream stream)
			: m_next(std::move(next))
			, m_stream(std::move(stream))
		{
		}

#if SILICIUM_COMPILER_GENERATES_MOVES
		zlib_deflating_sink(zlib_deflating_sink &&other) = default;
#else
		zlib_deflating_sink(zlib_deflating_sink &&other)
			: m_next(std::move(other.m_next))
			, m_stream(std::move(other.m_stream))
		{
		}
#endif

		boost::system::error_code append(iterator_range<element_type const *> data)
		{
			for (element_type const &piece : data)
			{
				auto piece_content_and_flag = visit<std::pair<iterator_range<char const *>, int>>(
					piece,
					[](flush)
					{
						return std::make_pair(iterator_range<char const *>(), Z_FULL_FLUSH /* ?? */);
					},
					[](iterator_range<char const *> content)
					{
						return std::make_pair(content, Z_NO_FLUSH);
					}
				);
				char const *next_in = piece_content_and_flag.first.begin();
				do
				{
					auto rest = make_iterator_range(next_in, piece_content_and_flag.first.end());
					Si::iterator_range<char *> buffer = m_next.make_append_space(std::max<size_t>(static_cast<std::size_t>(rest.size()), 4096));
					assert(!buffer.empty());
					auto result = m_stream.deflate(rest, buffer, piece_content_and_flag.second);
					std::size_t written = static_cast<std::size_t>(buffer.size()) - result.second;
					m_next.make_append_space(written);
					m_next.flush_append_space();
					next_in += (static_cast<std::size_t>(rest.size()) - result.first);
				}
				while (next_in != piece_content_and_flag.first.end());
			}
			return {};
		}

	private:

		Next m_next;
		zlib_deflate_stream m_stream;
	};

	template <class Next>
	auto make_deflating_sink(Next &&next, zlib_deflate_stream stream)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> zlib_deflating_sink<typename std::decay<Next>::type>
#endif
	{
		return zlib_deflating_sink<typename std::decay<Next>::type>(std::forward<Next>(next), std::move(stream));
	}
}
#endif

#endif
