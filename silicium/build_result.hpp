#ifndef SILICIUM_BUILD_RESULT_HPP
#define SILICIUM_BUILD_RESULT_HPP

#include <string>
#include <boost/variant.hpp>

namespace Si
{
	struct build_success
	{
	};

	struct build_failure
	{
		std::string short_description;
	};

	typedef boost::variant<build_success, build_failure> build_result;
}

#endif
