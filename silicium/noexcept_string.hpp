#ifndef SILICIUM_NOEXCEPT_STRING_HPP
#define SILICIUM_NOEXCEPT_STRING_HPP

#include <silicium/config.hpp>
#include <string>

#ifndef _MSC_VER
#	include <boost/container/string.hpp>
#endif

namespace Si
{
#ifdef _MSC_VER
	//boost string does not work at all on VC++ 2013 Update 3, so we use std::string instead
	typedef std::string noexcept_string;
	
	inline noexcept_string &&to_noexcept_string(noexcept_string &&str)
	{
		return std::move(str);
	}

	inline noexcept_string to_noexcept_string(noexcept_string const &str)
	{
		return str;
	}
#else

#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7)) || (__GNUC__ >= 5))
	typedef std::string noexcept_string;

	inline noexcept_string to_noexcept_string(boost::container::string const &str)
	{
		return noexcept_string(str.data(), str.size());
	}
#else
	typedef boost::container::string noexcept_string;

	inline noexcept_string to_noexcept_string(std::string const &str)
	{
		return noexcept_string(str.data(), str.size());
	}
#endif
	inline noexcept_string &&to_noexcept_string(noexcept_string &&str)
	{
		return std::move(str);
	}

	inline noexcept_string to_noexcept_string(noexcept_string const &str)
	{
		return str;
	}
#endif

	inline noexcept_string to_utf8_string(char const *utf8)
	{
		return utf8;
	}

	inline noexcept_string to_utf8_string(std::string const &utf8)
	{
#ifdef _WIN32
		return utf8;
#else
		return noexcept_string(utf8.begin(), utf8.end());
#endif
	}
}

#endif
