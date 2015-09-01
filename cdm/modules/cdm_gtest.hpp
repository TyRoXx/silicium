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

	inline Si::relative_path make_static_lib_build_path(Si::path_segment const &name_base)
	{
#ifdef _WIN32
		return Si::relative_path(L"Debug") / (name_base + *Si::path_segment::create(L".lib"));
#else
		return Si::relative_path("lib" + name_base.underlying() + ".a");
#endif
	}

	inline Si::relative_path make_static_lib_install_path(Si::path_segment const &name_base)
	{
#ifdef _WIN32
		return Si::relative_path(name_base + *Si::path_segment::create(L".lib"));
#else
		return Si::relative_path("lib" + name_base.underlying() + ".a");
#endif
	}

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
		Si::recreate_directories(lib_dir, Si::throw_);
		result.library = lib_dir / make_static_lib_install_path(*Si::path_segment::create("gtest"));
		result.library_main = lib_dir / make_static_lib_install_path(*Si::path_segment::create("gtest_main"));

		Si::copy(temporarily_writable / make_static_lib_build_path(*Si::path_segment::create("gtest")), result.library, boost::filesystem::copy_option::fail_if_exists, Si::throw_);
		Si::copy(temporarily_writable / make_static_lib_build_path(*Si::path_segment::create("gtest_main")), result.library_main, boost::filesystem::copy_option::fail_if_exists, Si::throw_);
		Si::remove_all(result.include, Si::throw_);
		Si::copy_recursively(gtest_source / *Si::path_segment::create("include"), result.include, &output, Si::throw_);

		return std::move(result);
	}
}

#endif
