#include <silicium/dynamic_library.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
	Si::dynamic_library load_libm()
	{
		return Si::dynamic_library("libm.so");
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
