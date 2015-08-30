#ifndef CDM_GTEST_HPP
#define CDM_GTEST_HPP

#include <silicium/file_operations.hpp>
#include <silicium/run_process.hpp>
#include <silicium/sink/ostream_sink.hpp>

namespace cdm
{
	struct gtest_paths
	{
		Si::absolute_path include;
		Si::absolute_path library;
		Si::absolute_path library_main;
	};

	inline gtest_paths install_gtest(
		Si::absolute_path const &gtest_source,
		Si::absolute_path const &temporarily_writable,
		Si::absolute_path const &install_root,
		Si::absolute_path const &cmake_exe)
	{
		auto output = Si::Sink<char, Si::success>::erase(Si::ostream_ref_sink(std::cerr));
		{
			std::vector<Si::os_string> arguments;
			arguments.push_back(gtest_source.c_str());
			int rc = Si::run_process(cmake_exe, arguments, temporarily_writable, output).get();
			if (rc != 0)
			{
				throw std::runtime_error("cmake configure failed");
			}
		}
		{
			std::vector<Si::os_string> arguments;
			arguments.push_back(SILICIUM_SYSTEM_LITERAL("--build"));
			arguments.push_back(SILICIUM_SYSTEM_LITERAL("."));
			int rc = Si::run_process(cmake_exe, arguments, temporarily_writable, output).get();
			if (rc != 0)
			{
				throw std::runtime_error("cmake build failed");
			}
		}
		gtest_paths result;
		result.include = install_root / *Si::path_segment::create("include");
		auto lib_dir = install_root / *Si::path_segment::create("lib");
		Si::recreate_directories(lib_dir);
		result.library = lib_dir / *Si::path_segment::create("libgtest.a");
		result.library_main = lib_dir / *Si::path_segment::create("libgtest_main.a");

		Si::throw_if_error(Si::copy(temporarily_writable / *Si::path_segment::create("libgtest.a"), result.library));
		Si::throw_if_error(Si::copy(temporarily_writable / *Si::path_segment::create("libgtest_main.a"), result.library_main));
		Si::remove_all(result.include).move_value();
		Si::copy_recursively(gtest_source / *Si::path_segment::create("include"), result.include, &output);

		return std::move(result);
	}
}

#endif
