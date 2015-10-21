#ifndef SILICIUM_WIN32_HPP
#define SILICIUM_WIN32_HPP

#include <memory>
#include <algorithm>
// asio does not like to be included after Windows headers
#include <boost/asio.hpp>
#include <Windows.h>

namespace Si
{
	namespace win32
	{
		struct handle_closer
		{
			void operator()(HANDLE h) const
			{
				CloseHandle(h);
			}
		};

		typedef std::unique_ptr<std::remove_pointer<HANDLE>::type, handle_closer> unique_handle;
	}
}

#endif
