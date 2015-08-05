#ifndef SILICIUM_NOEXCEPT_STRING_HPP
#define SILICIUM_NOEXCEPT_STRING_HPP

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
	typedef boost::container::string noexcept_string;

	inline noexcept_string to_noexcept_string(std::string const &str)
	{
		return noexcept_string(str.data(), str.size());
	}

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
}

#endif
