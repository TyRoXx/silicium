#ifndef SILICIUM_OS_STRING_HPP
#define SILICIUM_OS_STRING_HPP

#include <silicium/noexcept_string.hpp>
#include <silicium/throw_last_error.hpp>
#ifdef _WIN32
#include <silicium/win32/win32.hpp>
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

#ifndef _WIN32
	inline os_string to_os_string(std::string const &original)
	{
		return os_string(original.begin(), original.end());
	}

	inline os_string to_os_string(boost::container::string const &original)
	{
		return os_string(original.begin(), original.end());
	}

	inline os_string to_os_string(char const *original)
	{
		return original;
	}

	inline os_string to_os_string(char const *begin, char const *end)
	{
		return os_string(begin, end);
	}
#endif

#ifdef _WIN32
	namespace win32
	{
		inline winapi_string utf8_to_winapi_string(char const *original,
		                                           size_t length)
		{
			if (length > static_cast<size_t>((std::numeric_limits<int>::max)()))
			{
				throw std::invalid_argument(
				    "Input string is too long for WinAPI");
			}
			int const output_size = MultiByteToWideChar(
			    CP_UTF8, 0, original, static_cast<int>(length), nullptr, 0);
			assert(output_size >= 0);
			if ((length > 0) && (output_size == 0))
			{
				throw_last_error();
			}
			winapi_string result;
			result.resize(static_cast<size_t>(output_size));
			if (!result.empty())
			{
				MultiByteToWideChar(CP_UTF8, 0, original,
				                    static_cast<int>(length), &result[0],
				                    output_size);
			}
			return result;
		}

		inline noexcept_string to_utf8_string(wchar_t const *utf16,
		                                      size_t length)
		{
			if (length == 0)
			{
				// because WideCharToMultiByte fails for empty input
				return std::string();
			}
			if (length > static_cast<size_t>((std::numeric_limits<int>::max)()))
			{
				throw std::invalid_argument(
				    "Input string is too long for WinAPI");
			}
			int const destination_length =
			    WideCharToMultiByte(CP_UTF8, 0, utf16, static_cast<int>(length),
			                        nullptr, 0, 0, FALSE);
			if (!destination_length)
			{
				throw_last_error();
			}
			std::string result;
			result.resize(destination_length);
			if (!WideCharToMultiByte(CP_UTF8, 0, utf16,
			                         static_cast<int>(length), &result.front(),
			                         destination_length, 0, FALSE))
			{
				throw_last_error();
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

	inline os_string to_os_string(char const *begin, char const *end)
	{
		return win32::utf8_to_winapi_string(begin, end - begin);
	}

	inline noexcept_string to_utf8_string(os_string const &str)
	{
		return win32::to_utf8_string(str.data(), str.length());
	}

	inline noexcept_string to_utf8_string(wchar_t const *utf16)
	{
		return win32::to_utf8_string(utf16, std::wcslen(utf16));
	}
#endif
}

#endif
