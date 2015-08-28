#ifndef CDM_CPPNETLIB_HPP
#define CDM_CPPNETLIB_HPP

#include <silicium/file_operations.hpp>
#include <silicium/run_process.hpp>
#include <silicium/sink/ostream_sink.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>

namespace cdm
{
	struct cppnetlib_paths
	{
		Si::absolute_path include;
		Si::absolute_path uri_library;
		Si::absolute_path server_parsers_library;
		Si::absolute_path client_connections_library;
	};

	inline cppnetlib_paths install_cppnetlib(
		Si::absolute_path const &cppnetlib_source,
		Si::absolute_path const &temporarily_writable,
		Si::absolute_path const &install_root)
	{
		Si::absolute_path const cmake_exe = *Si::absolute_path::create("/usr/bin/cmake");
		auto output = Si::Sink<char, Si::success>::erase(Si::ostream_ref_sink(std::cerr));
		{
			std::vector<Si::noexcept_string> arguments;
			arguments.push_back(cppnetlib_source.c_str());
			arguments.push_back(("-DCMAKE_INSTALL_PREFIX=" + install_root.to_boost_path().string()).c_str());
			arguments.push_back("-DCPP-NETLIB_BUILD_SHARED_LIBS=ON");
			int rc = Si::run_process(cmake_exe.to_boost_path(), arguments, temporarily_writable.to_boost_path(), output);
			if (rc != 0)
			{
				throw std::runtime_error("cmake configure failed");
			}
		}
		{
			std::vector<Si::noexcept_string> arguments;
			arguments.push_back("--build");
			arguments.push_back(".");
			arguments.push_back("--");
			arguments.push_back("-j" + boost::lexical_cast<Si::noexcept_string>(boost::thread::hardware_concurrency()));
			arguments.push_back("install");
			int rc = Si::run_process(cmake_exe.to_boost_path(), arguments, temporarily_writable.to_boost_path(), output);
			if (rc != 0)
			{
				throw std::runtime_error("cmake build failed");
			}
		}
		cppnetlib_paths result;
		result.include = install_root / *Si::path_segment::create("include");
		auto lib_dir = install_root / *Si::path_segment::create("lib") / *Si::path_segment::create("x86_64-linux-gnu");
		result.uri_library = lib_dir / *Si::path_segment::create("libcppnetlib-uri.so");
		result.server_parsers_library = lib_dir / *Si::path_segment::create("libcppnetlib-server-parsers.so");
		result.client_connections_library = lib_dir / *Si::path_segment::create("libcppnetlib-client-connections.so");
		return std::move(result);
	}
}

#endif
