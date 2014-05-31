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

		build_failure() {}
		explicit build_failure(std::string short_description)
			: short_description(std::move(short_description))
		{
		}
	};

	typedef boost::variant<build_success, build_failure> build_result;

	struct result_to_short_string_converter : boost::static_visitor<std::string>
	{
		std::string operator()(Si::build_success const &) const
		{
			return "success";
		}

		std::string operator()(Si::build_failure const &) const
		{
			return "failure";
		}
	};

	inline std::string to_short_string(build_result const &result)
	{
		return boost::apply_visitor(result_to_short_string_converter{}, result);
	}

	struct result_to_long_string_converter : boost::static_visitor<std::string>
	{
		std::string operator()(Si::build_success const &) const
		{
			return "success";
		}

		std::string operator()(Si::build_failure const &failure) const
		{
			return "failure(" + failure.short_description + ")";
		}
	};

	inline std::string to_long_string(build_result const &result)
	{
		return boost::apply_visitor(result_to_long_string_converter{}, result);
	}
}

#endif
