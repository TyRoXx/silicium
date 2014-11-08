#include <silicium/sink.hpp>
#include <silicium/source.hpp>
#include <silicium/fast_variant.hpp>
#include <silicium/container_buffer.hpp>
#include <boost/test/unit_test.hpp>
#include <zlib.h>

namespace Si
{
	struct zlib_error_category : boost::system::error_category
	{
		virtual const char *name() const BOOST_SYSTEM_NOEXCEPT SILICIUM_OVERRIDE
		{
			return "zlib";
		}

        virtual std::string message(int ev) const SILICIUM_OVERRIDE
		{
			switch (ev)
			{
#define SILICIUM_HANDLE_ERROR_CASE(error) case error: return BOOST_STRINGIZE(error);
			SILICIUM_HANDLE_ERROR_CASE(Z_ERRNO)
			SILICIUM_HANDLE_ERROR_CASE(Z_STREAM_ERROR)
			SILICIUM_HANDLE_ERROR_CASE(Z_DATA_ERROR)
			SILICIUM_HANDLE_ERROR_CASE(Z_MEM_ERROR)
			SILICIUM_HANDLE_ERROR_CASE(Z_BUF_ERROR)
			SILICIUM_HANDLE_ERROR_CASE(Z_VERSION_ERROR)
#undef SILICIUM_HANDLE_ERROR_CASE
			default:
				return "";
			}
		}
	};

	boost::system::error_category const &zlib_category()
	{
		static zlib_error_category const instance;
		return instance;
	}

	void handle_zlib_status(int status)
	{
		if (status == Z_OK)
		{
			return;
		}
		throw boost::system::system_error(status, zlib_category());
	}

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
			swap(m_stream, other.m_stream);
		}

		zlib_deflate_stream &operator = (zlib_deflate_stream &&other) BOOST_NOEXCEPT
		{
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
			boost::iterator_range<char const *> original,
			boost::iterator_range<char *> deflated,
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
			return std::make_pair(m_stream->avail_in, m_stream->avail_out);
		}

	private:

		boost::optional<z_stream> m_stream;

		SILICIUM_DELETED_FUNCTION(zlib_deflate_stream(zlib_deflate_stream const &))
		SILICIUM_DELETED_FUNCTION(zlib_deflate_stream &operator = (zlib_deflate_stream const &))
	};

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
			swap(m_stream, other.m_stream);
		}

		zlib_inflate_stream &operator = (zlib_inflate_stream &&other) BOOST_NOEXCEPT
		{
			swap(m_stream, other.m_stream);
			return *this;
		}

		std::pair<std::size_t, std::size_t> inflate(
			boost::iterator_range<char const *> deflated,
			boost::iterator_range<char *> original,
			int flush) BOOST_NOEXCEPT
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

		boost::optional<z_stream> m_stream;

		explicit zlib_inflate_stream(z_stream stream) BOOST_NOEXCEPT
			: m_stream(stream)
		{
		}

		SILICIUM_DELETED_FUNCTION(zlib_inflate_stream(zlib_inflate_stream const &))
		SILICIUM_DELETED_FUNCTION(zlib_inflate_stream &operator = (zlib_inflate_stream const &))
	};

	BOOST_AUTO_TEST_CASE(zlib_stream_wrappers)
	{
		zlib_deflate_stream deflator(Z_DEFAULT_COMPRESSION);
		std::string const original = "Hello";
		std::array<char, 4096> compressed;
		std::pair<std::size_t, std::size_t> compress_result = deflator.deflate(
			boost::make_iterator_range(original.data(), original.data() + original.size()),
			boost::make_iterator_range(compressed.data(), compressed.data() + compressed.size()),
			Z_FULL_FLUSH
		);
		BOOST_CHECK_EQUAL(0, compress_result.first);
		BOOST_CHECK_LT(compress_result.second, compressed.size());
		std::array<char, 4096> decompressed;
		zlib_inflate_stream inflator = zlib_inflate_stream::initialize();
		std::pair<std::size_t, std::size_t> decompress_result = inflator.inflate(
			boost::make_iterator_range(compressed.data(), compressed.data() + compressed.size() - compress_result.second),
			boost::make_iterator_range(decompressed.data(), decompressed.data() + decompressed.size()),
			Z_SYNC_FLUSH
		);
		auto const decompressed_length = decompressed.size() - decompress_result.second;
		BOOST_REQUIRE_EQUAL(original.size(), decompressed_length);
		BOOST_CHECK_EQUAL(original, std::string(decompressed.begin(), decompressed.begin() + decompressed_length));
	}

	struct flush
	{
	};

	typedef fast_variant<flush, boost::iterator_range<char const *>> zlib_sink_element;

	template <class Next>
	struct zlib_deflating_sink : sink<zlib_sink_element>
	{
		typedef zlib_sink_element element_type;

		zlib_deflating_sink()
		{
		}

		explicit zlib_deflating_sink(Next next, zlib_deflate_stream stream)
			: m_next(std::move(next))
			, m_stream(std::move(stream))
		{
		}

		BOOST_DEFAULTED_FUNCTION(zlib_deflating_sink(zlib_deflating_sink &&other),
			: m_next(std::move(other.m_next))
			BOOST_PP_COMMA() m_stream(std::move(other.m_stream))
		{
		})

		virtual boost::system::error_code append(boost::iterator_range<element_type const *> data) SILICIUM_OVERRIDE
		{
			for (element_type const &piece : data)
			{
				auto piece_content_and_flag = visit<std::pair<boost::iterator_range<char const *>, int>>(
					piece,
					[](flush)
					{
						return std::make_pair(boost::iterator_range<char const *>(), Z_FULL_FLUSH /* ?? */);
					},
					[](boost::iterator_range<char const *> content)
					{
						return std::make_pair(content, Z_NO_FLUSH);
					}
				);
				char const *next_in = piece_content_and_flag.first.begin();
				do
				{
					auto rest = boost::make_iterator_range(next_in, piece_content_and_flag.first.end());
					boost::iterator_range<char *> buffer = m_next.make_append_space(std::max<size_t>(rest.size(), 4096));
					assert(!buffer.empty());
					auto result = m_stream.deflate(rest, buffer, piece_content_and_flag.second);
					std::size_t written = buffer.size() - result.second;
					m_next.make_append_space(written);
					m_next.flush_append_space();
					next_in += (rest.size() - result.first);
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
	{
		return zlib_deflating_sink<typename std::decay<Next>::type>(std::forward<Next>(next), std::move(stream));
	}

	template <class C>
	auto make_c_str_range(C const *str)
	{
		return boost::make_iterator_range(str, str + std::char_traits<C>::length(str));
	}

	BOOST_AUTO_TEST_CASE(zlib_sink)
	{
		std::vector<char> compressed;
		auto compressor = make_deflating_sink(make_container_buffer(compressed), zlib_deflate_stream(Z_DEFAULT_COMPRESSION));
		append(compressor, zlib_sink_element{make_c_str_range("Hello")});
		append(compressor, zlib_sink_element{flush{}});
		BOOST_CHECK_GE(compressed.size(), 1);
	}
}
