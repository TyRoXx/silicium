#include <silicium/error_or.hpp>
#include <boost/test/unit_test.hpp>
#include <system_error>

BOOST_AUTO_TEST_CASE(error_or_no_throw)
{
	Si::error_or<int> value(2);
	BOOST_CHECK(!value.is_error());
	BOOST_REQUIRE(value.get_ptr());
	BOOST_CHECK_EQUAL(2, *value.get_ptr());
	BOOST_CHECK_EQUAL(2, value.get());
}

BOOST_AUTO_TEST_CASE(error_or_movable_only)
{
	Si::error_or<std::unique_ptr<int>> value(Si::make_unique<int>(2));
	BOOST_REQUIRE(value.get_ptr());
	BOOST_REQUIRE(*value.get_ptr());
	BOOST_CHECK_EQUAL(2, **value.get_ptr());
	std::unique_ptr<int> v = std::move(value).get();
	BOOST_CHECK(!value.get());
	BOOST_CHECK(!value.is_error());
	BOOST_CHECK_EQUAL(2, *v);
}

BOOST_AUTO_TEST_CASE(error_or_const)
{
	Si::error_or<int> const value(2);
	BOOST_CHECK(!value.is_error());
	BOOST_CHECK_EQUAL(2, value.get());
	BOOST_REQUIRE(value.get_ptr());
	BOOST_CHECK_EQUAL(2, *value.get_ptr());
}

BOOST_AUTO_TEST_CASE(error_or_throw)
{
	boost::system::error_code const ec(123, boost::system::native_ecat);
	Si::error_or<int> error(ec);
	BOOST_CHECK(error.is_error());
	BOOST_CHECK_EXCEPTION(error.get(), boost::system::system_error, [ec](boost::system::system_error const &ex)
	{
		return ex.code() == ec;
	});
}

BOOST_AUTO_TEST_CASE(error_or_std)
{
	std::error_code const ec(123, std::system_category());
	Si::error_or<int, std::error_code> error(ec);
	BOOST_CHECK(error.is_error());
	BOOST_CHECK_EXCEPTION(error.get(), std::system_error, [ec](std::system_error const &ex)
	{
		return ex.code() == ec;
	});
}

struct base
{
	virtual ~base()
	{
	}
};

struct derived : base
{
};

BOOST_AUTO_TEST_CASE(error_or_construct_from_convertible)
{
	Si::error_or<long> e(2u);
	BOOST_CHECK_EQUAL(2L, e.get());
	Si::error_or<std::unique_ptr<base>> f(Si::make_unique<derived>());
	BOOST_CHECK(f.get() != nullptr);
}

BOOST_AUTO_TEST_CASE(error_or_map_value)
{
	BOOST_CHECK_EQUAL(Si::error_or<long>(3L), Si::map(Si::error_or<long>(2L), [](long value)
	{
		return value + 1;
	}));
}

BOOST_AUTO_TEST_CASE(error_or_map_error)
{
	BOOST_CHECK_EQUAL(Si::error_or<long>(boost::system::error_code()), Si::map(Si::error_or<long>(boost::system::error_code()), [](long)
	{
		BOOST_FAIL("no value expected");
		return 0;
	}));
}

BOOST_AUTO_TEST_CASE(error_or_equal)
{
	{
		Si::error_or<int> a(2);
		Si::error_or<int> b = a;
		BOOST_CHECK_EQUAL(a, a);
		BOOST_CHECK_EQUAL(a, b);
		BOOST_CHECK_EQUAL(b, a);
		BOOST_CHECK_EQUAL(b, b);
	}

	{
		Si::error_or<int> c = boost::system::error_code();
		Si::error_or<int> d = c;
		BOOST_CHECK_EQUAL(c, c);
		BOOST_CHECK_EQUAL(c, d);
		BOOST_CHECK_EQUAL(d, c);
		BOOST_CHECK_EQUAL(d, d);
	}
}

BOOST_AUTO_TEST_CASE(error_or_not_equal)
{
	Si::error_or<int> a(2);
	Si::error_or<int> b(3);
	Si::error_or<int> c = boost::system::error_code(2, boost::system::generic_category());
	Si::error_or<int> d = boost::system::error_code(3, boost::system::generic_category());

	BOOST_CHECK_NE(a, b);
	BOOST_CHECK_NE(a, c);
	BOOST_CHECK_NE(a, d);

	BOOST_CHECK_NE(b, a);
	BOOST_CHECK_NE(b, c);
	BOOST_CHECK_NE(b, d);

	BOOST_CHECK_NE(c, a);
	BOOST_CHECK_NE(c, b);
	BOOST_CHECK_NE(c, d);

	BOOST_CHECK_NE(d, a);
	BOOST_CHECK_NE(d, b);
	BOOST_CHECK_NE(d, c);
}
