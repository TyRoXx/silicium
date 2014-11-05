#include <silicium/sink.hpp>
#include <silicium/source.hpp>
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
			assert(original.begin());
			assert(original.size());
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

	BOOST_AUTO_TEST_CASE(zlib_deflate)
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
}
