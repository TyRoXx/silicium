#ifndef SILICIUM_OS_STRING_HPP
#define SILICIUM_OS_STRING_HPP

#include <silicium/noexcept_string.hpp>
#ifdef _WIN32
#	include <silicium/win32/win32.hpp>
#endif
#include <string>

namespace Si
{
	typedef
#ifdef _WIN32
		wchar_t
#else
		char
#endif
		os_char;

#ifdef _WIN32
	typedef std::basic_string<os_char> os_string;
	namespace win32
	{
		typedef os_string winapi_string;
	}
#else
	typedef noexcept_string os_string;
#endif

	inline os_string to_os_string(os_string original)
	{
		return original;
	}

#ifdef _WIN32
	namespace win32
	{
		inline winapi_string utf8_to_winapi_string(char const *original, size_t length)
		{
			if (length > static_cast<size_t>((std::numeric_limits<int>::max)()))
			{
				throw std::invalid_argument("Input string is too long for WinAPI");
			}
			int const output_size = MultiByteToWideChar(CP_UTF8, 0, original, static_cast<int>(length), nullptr, 0);
			assert(output_size >= 0);
			if (output_size == 0)
			{
				assert(GetLastError() == ERROR_NO_UNICODE_TRANSLATION);
				throw std::invalid_argument("Input string is not UTF-8");
			}
			winapi_string result;
			result.resize(static_cast<size_t>(output_size));
			if (!result.empty())
			{
				MultiByteToWideChar(CP_UTF8, 0, original, static_cast<int>(length), &result[0], output_size);
			}
			return result;
		}
	}

	inline os_string to_os_string(char const *c_str)
	{
		return win32::utf8_to_winapi_string(c_str, std::strlen(c_str));
	}

	inline os_string to_os_string(noexcept_string const &original)
	{
		return win32::utf8_to_winapi_string(original.data(), original.size());
	}
#endif
}

#endif
