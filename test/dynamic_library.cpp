#include <silicium/dynamic_library.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
	Si::dynamic_library load_libm()
	{
		return Si::dynamic_library(Si::native_path_string(
#ifdef _WIN32
			L"msvcr110.dll" //assuming VC++ 2013
#elif defined(__APPLE__)
			"libm.dylib"
#else
			"libm.so"
#endif
			));
	}

	Si::dynamic_library load_some_library()
	{
		return load_libm();
	}
}

BOOST_AUTO_TEST_CASE(dynamic_library_find_symbol)
{
	Si::dynamic_library lib = load_libm();
	BOOST_REQUIRE(!lib.empty());
	void *sin_ = lib.find_symbol(Si::c_string("sin"));
	BOOST_REQUIRE(sin_);
	auto const sin_callable = Si::function_ptr_cast<double (*)(double)>(sin_);
	BOOST_CHECK_EQUAL(0, sin_callable(0));
}

BOOST_AUTO_TEST_CASE(dynamic_library_move)
{
	Si::dynamic_library lib = load_some_library();
	BOOST_CHECK(!lib.empty());
	Si::dynamic_library moved = std::move(lib);
	BOOST_CHECK(lib.empty());
	BOOST_CHECK(!moved.empty());
	lib = std::move(moved);
	BOOST_CHECK(!lib.empty());
	BOOST_CHECK(moved.empty());
}
