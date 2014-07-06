#include <silicium/fast_variant.hpp>
#include <boost/container/string.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(fast_variant_single)
	{
		fast_variant<int> v;
		BOOST_CHECK_EQUAL(0, v.which());
	}

	BOOST_AUTO_TEST_CASE(fast_variant_assignment_same)
	{
		fast_variant<int, boost::container::string> v, w;
		v = w;
	}

	BOOST_AUTO_TEST_CASE(fast_variant_assignment_different)
	{
		fast_variant<int, boost::container::string> v, w(boost::container::string(""));
		v = w;
	}
}
