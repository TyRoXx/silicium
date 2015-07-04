#ifndef SILICIUM_TERMINATE_ON_EXCEPTION_HPP
#define SILICIUM_TERMINATE_ON_EXCEPTION_HPP

#include <boost/throw_exception.hpp>
#include <boost/concept_check.hpp>

namespace boost
{
#ifdef BOOST_NO_EXCEPTIONS
	void throw_exception(std::exception const &e)
	{
		boost::ignore_unused_variable_warning(e);
		std::terminate();
	}
#endif
}

#endif
