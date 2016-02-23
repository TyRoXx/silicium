#ifndef SILICIUM_EXPLICIT_OPERATOR_BOOL_HPP
#define SILICIUM_EXPLICIT_OPERATOR_BOOL_HPP

#include <boost/version.hpp>
#if BOOST_VERSION >= 105500
#include <boost/utility/explicit_operator_bool.hpp>
#endif

#ifdef BOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT
// the noexcept version was added in 1.56
// http://www.boost.org/doc/libs/1_57_0/libs/core/doc/html/core/explicit_operator_bool.html
#define SILICIUM_EXPLICIT_OPERATOR_BOOL()                                      \
	BOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()

#elif defined(BOOST_EXPLICIT_OPERATOR_BOOL)
#define SILICIUM_EXPLICIT_OPERATOR_BOOL() BOOST_EXPLICIT_OPERATOR_BOOL()

#else
#define SILICIUM_EXPLICIT_OPERATOR_BOOL()                                      \
	operator bool() const BOOST_NOEXCEPT                                       \
	{                                                                          \
		return !!(*this);                                                      \
	}
#endif

#endif
