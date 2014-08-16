#include <silicium/dynamic_library.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(dynamic_library_find_symbol)
{
	Si::dynamic_library lib("libc.so");
	void *malloc_ = lib.find_symbol("malloc");
	BOOST_REQUIRE(malloc_);
	void *free_ = lib.find_symbol("free");
	BOOST_REQUIRE(free_);
	std::size_t const size = 10000;
	char * const allocated = static_cast<char *>(((void *(*)(std::size_t))malloc_)(size));
	BOOST_REQUIRE(allocated);
	std::fill(allocated, allocated + size, '-');
	((void (*)(void *))free_)(allocated);
}
