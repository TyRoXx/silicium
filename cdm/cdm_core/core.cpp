#include "core.hpp"
#include <silicium/expected.hpp>

namespace cdm
{
	error_category const &get_error_category()
	{
		static error_category instance;
		return instance;
	}

	Si::error_or<dynamic_library_description> load_dynamic_library_description(Si::absolute_path const &file)
	{
		dynamic_library_description result;
		result.library.open(Si::native_path_string(file.c_str()));
		auto const describe = Si::function_ptr_cast<void (*)(Si::expected<description> *)>(result.library.find_symbol(Si::c_string("cdm_describe")));
		if (!describe)
		{
			return boost::system::error_code(error::cdm_describe_not_found);
		}
		Si::expected<description> maybe_description;
		describe(&maybe_description);
		result.cached_description = std::move(maybe_description).value();
		return Si::error_or<dynamic_library_description>(std::move(result));
	}
}
