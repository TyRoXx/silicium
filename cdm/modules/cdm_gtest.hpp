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
		Si::absolute_path const gtest_in_cache = install_root / Si::relative_path("gtest");
		Si::relative_path const gtest_lib_name = make_static_lib_build_path(*Si::path_segment::create("gtest"));
		Si::relative_path const gtest_main_lib_name = make_static_lib_build_path(*Si::path_segment::create("gtest_main"));
		if (!Si::file_exists(gtest_in_cache, Si::throw_))
		{
			Si::absolute_path const build_dir = temporarily_writable / Si::relative_path("build");
			Si::create_directories(build_dir, Si::throw_);
			auto output = Si::Sink<char, Si::success>::erase(Si::ostream_ref_sink(std::cerr));
			{
				std::vector<Si::os_string> arguments;
				arguments.push_back(gtest_source.c_str());
				int rc = Si::run_process(cmake_exe, arguments, build_dir, output).get();
				if (rc != 0)
				{
					throw std::runtime_error("cmake configure failed");
				}
			}
			{
				std::vector<Si::os_string> arguments;
				arguments.push_back(SILICIUM_SYSTEM_LITERAL("--build"));
				arguments.push_back(SILICIUM_SYSTEM_LITERAL("."));
				int rc = Si::run_process(cmake_exe, arguments, build_dir, output).get();
				if (rc != 0)
				{
					throw std::runtime_error("cmake build failed");
				}
			}
			Si::absolute_path const construction_site = temporarily_writable / Si::relative_path("construction");
			Si::create_directories(construction_site, Si::throw_);
			{
				Si::absolute_path const lib_dir = construction_site / Si::relative_path("lib");
				Si::create_directories(lib_dir, Si::throw_);
				Si::copy(build_dir / gtest_lib_name, lib_dir / gtest_lib_name, Si::throw_);
				Si::copy(build_dir / gtest_main_lib_name, lib_dir / gtest_main_lib_name, Si::throw_);
				Si::copy_recursively(gtest_source / *Si::path_segment::create("include"), construction_site / Si::relative_path("include"), &output, Si::throw_);
			}
			Si::rename(construction_site, gtest_in_cache, Si::throw_);
		}
		gtest_paths result;
		result.include = gtest_in_cache / *Si::path_segment::create("include");
		auto lib_dir = gtest_in_cache / *Si::path_segment::create("lib");
		result.library = lib_dir / gtest_lib_name;
		result.library_main = lib_dir / gtest_main_lib_name;
		return std::move(result);
	}
}

#endif
