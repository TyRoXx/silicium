#include <cdm_core/core.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
	Si::error_or<Si::absolute_path> get_current_executable_path()
	{
#ifdef _WIN32
		//will be enough for most cases
		std::vector<wchar_t> buffer(MAX_PATH);
		for (;;)
		{
			auto const length = GetModuleFileNameW(NULL, buffer.data(), buffer.size());
			auto const error = Si::get_last_error();
			switch (error.value())
			{
			case ERROR_INSUFFICIENT_BUFFER:
				buffer.resize(buffer.size() * 2);
				break;

			case ERROR_SUCCESS:
			{
				boost::filesystem::path path(buffer.begin(), buffer.begin() + length);
				return *Si::absolute_path::create(std::move(path));
			}

			default:
				return error;
			}
		}
#else
		boost::system::error_code ec;
		auto result = boost::filesystem::read_symlink("/proc/self/exe", ec);
		if (!!ec)
		{
			return ec;
		}
		return *Si::absolute_path::create(std::move(result));
#endif
	}
}

BOOST_AUTO_TEST_CASE(cdm_core_trivial)
{
	auto const directory_containing_test_exe = get_current_executable_path().get().to_boost_path().parent_path();
#ifdef _MSC_VER
	auto const build_type = directory_containing_test_exe.leaf();
#endif
	auto const description_library = directory_containing_test_exe.parent_path()
#ifdef _MSC_VER
		.parent_path()
#endif
		/ "cdm" / "websocketpp" /
#ifdef _MSC_VER
		build_type / "cdm_websocketpp.dll"
#else
		"libcdm_websocketpp.so"
#endif
		;
	cdm::dynamic_library_description const loaded = cdm::load_dynamic_library_description(*Si::absolute_path::create(description_library)).move_value();
	BOOST_CHECK_EQUAL("websocketpp", loaded.cached_description.name);
	BOOST_CHECK(!loaded.library.empty());
}
