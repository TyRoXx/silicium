#ifndef CDM_CPPNETLIB_HPP
#define CDM_CPPNETLIB_HPP

#include <silicium/file_operations.hpp>
#include <silicium/run_process.hpp>
#include <silicium/sink/ostream_sink.hpp>
#include <boost/lexical_cast.hpp>

namespace cdm
{
	struct cppnetlib_paths
	{
		Si::absolute_path cmake_prefix_path;
	};

	inline cppnetlib_paths install_cppnetlib(
		Si::absolute_path const &cppnetlib_source,
		Si::absolute_path const &install_root,
		Si::absolute_path const &cmake_exe,
		unsigned make_parallelism)
	{
		auto output = Si::Sink<char, Si::success>::erase(Si::ostream_ref_sink(std::cerr));
		{
			std::vector<Si::os_string> arguments;
			arguments.push_back(cppnetlib_source.c_str());
			arguments.push_back(SILICIUM_SYSTEM_LITERAL("-DCPP-NETLIB_BUILD_SHARED_LIBS=ON"));
#ifdef _WIN32
			//TODO: deal with OpenSSL later..
			arguments.push_back(SILICIUM_SYSTEM_LITERAL("-DCPP-NETLIB_ENABLE_HTTPS=OFF"));
#endif
			int const rc = Si::run_process(cmake_exe, arguments, install_root, output).get();
			if (rc != 0)
			{
				throw std::runtime_error("cmake configure failed");
			}
		}
		{
			std::vector<Si::os_string> arguments;
			arguments.push_back(SILICIUM_SYSTEM_LITERAL("--build"));
			arguments.push_back(SILICIUM_SYSTEM_LITERAL("."));
#ifndef _WIN32
			arguments.push_back(SILICIUM_SYSTEM_LITERAL("--"));
			arguments.push_back(SILICIUM_SYSTEM_LITERAL("-j" + boost::lexical_cast<Si::os_string>(make_parallelism)));
#else
			boost::ignore_unused_variable_warning(make_parallelism);
#endif
			int const rc = Si::run_process(cmake_exe, arguments, install_root, output).get();
			if (rc != 0)
			{
				throw std::runtime_error("cmake build failed");
			}
		}
		cppnetlib_paths result;
		result.cmake_prefix_path = install_root;
		return std::move(result);
	}
}

#endif
