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

	struct zlib_stream
	{
		zlib_stream() BOOST_NOEXCEPT
		{
		}

		explicit zlib_stream(int level)
		{
			z_stream zero;
			std::memset(&zero, 0, sizeof(zero));
			m_stream = zero;
			handle_zlib_status(deflateInit(&*m_stream, level));
		}

		~zlib_stream() BOOST_NOEXCEPT
		{
			if (!m_stream)
			{
				return;
			}
			deflateEnd(&*m_stream);
		}

	private:

		boost::optional<z_stream> m_stream;

		SILICIUM_DELETED_FUNCTION(zlib_stream(zlib_stream const &))
		SILICIUM_DELETED_FUNCTION(zlib_stream &operator = (zlib_stream const &))
	};

	BOOST_AUTO_TEST_CASE(zlib_deflate)
	{
		zlib_stream stream;
	}
}
