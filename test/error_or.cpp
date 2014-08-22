#include <silicium/error_or.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(error_or_no_throw)
{
	Si::error_or<int> value(2);
	BOOST_CHECK(!value.is_error());
	BOOST_CHECK_EQUAL(2, value.value());
}

BOOST_AUTO_TEST_CASE(error_or_movable_only)
{
	Si::error_or<std::unique_ptr<int>> value(Si::make_unique<int>(2));
	std::unique_ptr<int> v = std::move(value).value();
	BOOST_CHECK(!value.value());
	BOOST_CHECK(!value.is_error());
	BOOST_CHECK_EQUAL(2, *v);
}

BOOST_AUTO_TEST_CASE(error_or_const)
{
	Si::error_or<int> const value(2);
	BOOST_CHECK(!value.is_error());
	BOOST_CHECK_EQUAL(2, value.value());
}

BOOST_AUTO_TEST_CASE(error_or_throw)
{
	boost::system::error_code const ec(123, boost::system::native_ecat);
	Si::error_or<int> error(ec);
	BOOST_CHECK(error.is_error());
	BOOST_CHECK_EXCEPTION(error.value(), boost::system::system_error, [ec](boost::system::system_error const &ex)
	{
		return ex.code() == ec;
	});
}
