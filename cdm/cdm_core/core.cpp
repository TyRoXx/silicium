#include "core.hpp"

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
		boost::system::error_code ec;
		result.library.open(Si::native_path_string(file.c_str()), ec);
		if (!!ec)
		{
			return ec;
		}
		auto const describe = Si::function_ptr_cast<bool (*)(description *)>(result.library.find_symbol(Si::c_string("cdm_describe")));
		if (!describe)
		{
			return boost::system::error_code(error::cdm_describe_not_found);
		}
		if (!describe(&result.cached_description))
		{
			return boost::system::error_code(error::cdm_describe_failed);
		}
		return result;
	}
}
