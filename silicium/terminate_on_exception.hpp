#ifndef SILICIUM_TERMINATE_ON_EXCEPTION_HPP
#define SILICIUM_TERMINATE_ON_EXCEPTION_HPP

#include <silicium/config.hpp>
#include <boost/throw_exception.hpp>

namespace boost
{
#ifdef BOOST_NO_EXCEPTIONS
	void throw_exception(std::exception const &e)
	{
		Si::ignore_unused_variable_warning(e);
		std::terminate();
	}
#endif
}

#endif
