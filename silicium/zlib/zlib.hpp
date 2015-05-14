#ifndef SILICIUM_ZLIB_ZLIB_HPP
#define SILICIUM_ZLIB_ZLIB_HPP

#include <boost/system/system_error.hpp>
#include <zlib.h>

namespace Si
{
	struct zlib_error_category : boost::system::error_category
	{
		virtual const char *name() const BOOST_NOEXCEPT SILICIUM_OVERRIDE
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
}

#endif
