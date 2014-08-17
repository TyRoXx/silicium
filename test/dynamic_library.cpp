#include <silicium/dynamic_library.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
	Si::dynamic_library load_libm()
	{
		return Si::dynamic_library(
#ifdef _WIN32
			"msvcr110.dll" //assuming VC++ 2013
#else
			"libm.so"
#endif
			);
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
	void *sin_ = lib.find_symbol("sin");
	BOOST_REQUIRE(sin_);
#ifdef __GNUC__
	//silence Warnung: ISO C++ forbids casting between pointer-to-function and pointer-to-object [enabled by default]
	__extension__
#endif
	auto const sin_callable = reinterpret_cast<double (*)(double)>(sin_);
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
