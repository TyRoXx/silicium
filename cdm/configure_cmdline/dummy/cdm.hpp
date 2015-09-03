#include <silicium/absolute_path.hpp>

namespace CDM_CONFIGURE_NAMESPACE
{
	void configure(
		Si::absolute_path const &module_temporaries,
		Si::absolute_path const &module_permanent,
		Si::absolute_path const &application_source,
		Si::absolute_path const &application_build_dir,
		Si::Sink<char, Si::success>::interface &output
		)
	{
		boost::ignore_unused_variable_warning(module_temporaries);
		boost::ignore_unused_variable_warning(module_permanent);
		boost::ignore_unused_variable_warning(application_source);
		boost::ignore_unused_variable_warning(application_build_dir);
		boost::ignore_unused_variable_warning(output);
	}
}
