#ifndef CDM_BUILDING_CONFIGURE_HPP
#define CDM_BUILDING_CONFIGURE_HPP

#include <silicium/absolute_path.hpp>

namespace cdm
{
	void build_configure_command_line(
		Si::absolute_path const &application_source,
		Si::absolute_path const &temporary,
		Si::absolute_path const &resulting_executable,
		Si::Sink<char, Si::success>::interface &output
	);

	void run_configure_command_line(
		Si::absolute_path const &configure_executable,
		Si::absolute_path const &module_permanent,
		Si::absolute_path const &application_source,
		Si::absolute_path const &application_build_dir,
		Si::Sink<char, Si::success>::interface &output
	);
}

#endif
