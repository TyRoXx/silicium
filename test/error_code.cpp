#include <silicium/error_code.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/unordered_set.hpp>
#include <unordered_set>

BOOST_AUTO_TEST_CASE(error_code_copy)
{
	Si::error_code<> const e;
	Si::error_code<> const f = Si::error_code<>::create<1, boost::system::system_category>();
	auto a = e;
	BOOST_CHECK_EQUAL(a, e);
	a = f;
	BOOST_CHECK_EQUAL(a, f);
}

BOOST_AUTO_TEST_CASE(error_code_hash)
{
	boost::unordered_set<Si::error_code<>> m;
	std::unordered_set<Si::error_code<>> n;
	m.insert(n.begin(), n.end());
	n.insert(m.begin(), m.end());
}
