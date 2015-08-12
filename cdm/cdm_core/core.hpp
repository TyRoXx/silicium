#ifndef CDM_CORE_HPP
#define CDM_CORE_HPP

#include <cdm_description/all.hpp>
#include <silicium/absolute_path.hpp>
#include <silicium/dynamic_library.hpp>

namespace cdm
{
	struct dynamic_library_description
	{
		Si::dynamic_library library;
		description cached_description;
	};

	enum class error
	{
		cdm_describe_not_found = 1,
		cdm_describe_failed
	};

	struct error_category : boost::system::error_category
	{
		virtual const char *name() const BOOST_SYSTEM_NOEXCEPT SILICIUM_OVERRIDE
		{
			return "cdm";
		}

		virtual std::string message(int ev) const SILICIUM_OVERRIDE
		{
			switch (static_cast<error>(ev))
			{
			case error::cdm_describe_not_found: return "cdm_describe_not_found";
			case error::cdm_describe_failed: return "cdm_describe_failed";
			}
			return "";
		}
	};

	error_category const &get_error_category();

	inline boost::system::error_code make_error_code(error value)
	{
		return boost::system::error_code(static_cast<int>(value), get_error_category());
	}

	Si::error_or<dynamic_library_description> load_dynamic_library_description(Si::absolute_path const &file);
}

namespace boost
{
	namespace system
	{
		template <>
		struct is_error_code_enum<cdm::error> : std::true_type {};
	}
}

#endif
