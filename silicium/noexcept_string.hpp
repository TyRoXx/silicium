#ifndef SILICIUM_NOEXCEPT_STRING_HPP
#define SILICIUM_NOEXCEPT_STRING_HPP

#ifdef _MSC_VER
#	include <string>
#else
#	include <boost/container/string.hpp>
#endif

namespace Si
{
#ifdef _MSC_VER
	//boost string does not work at all on VC++ 2013 Update 3, so we use std::string instead
	typedef std::string noexcept_string;
#else
	typedef boost::container::string noexcept_string;
#endif
}

#endif
