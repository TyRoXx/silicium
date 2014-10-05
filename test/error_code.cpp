#include <silicium/error_code.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/unordered_set.hpp>
#include <unordered_set>

BOOST_AUTO_TEST_CASE(error_code_default_constructor)
{
	Si::error_code<> const e;
	BOOST_CHECK_EQUAL(boost::system::error_code(), e.to_underlying());
	BOOST_CHECK_EQUAL(e, e);
	BOOST_CHECK_EQUAL(0, e.value());
	BOOST_CHECK_EQUAL(&boost::system::error_code().category(), &e.category());
}

BOOST_AUTO_TEST_CASE(error_code_create)
{
	Si::error_code<> const e = Si::error_code<>::create<1, boost::system::system_category>();
	BOOST_CHECK_EQUAL(boost::system::error_code(1, boost::system::system_category()), e.to_underlying());
	BOOST_CHECK_EQUAL(e, e);
	BOOST_CHECK_EQUAL(1, e.value());
	BOOST_CHECK_EQUAL(&boost::system::system_category(), &e.category());
}

BOOST_AUTO_TEST_CASE(error_code_clear)
{
	Si::error_code<> e = Si::error_code<>::create<1, boost::system::system_category>();
	e.clear();
	BOOST_CHECK_EQUAL(Si::error_code<>(), e);
}

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
