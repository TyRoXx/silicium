#include <silicium/expected.hpp>
#include <boost/test/unit_test.hpp>

#if SILICIUM_HAS_EXPECTED
BOOST_AUTO_TEST_CASE(expected_default_construct)
{
	Si::expected<int> e;
	BOOST_CHECK(e.valid());
	BOOST_CHECK_EQUAL(0, e.value());
}

BOOST_AUTO_TEST_CASE(expected_construct_value)
{
	Si::expected<int> e(12);
	BOOST_CHECK(e.valid());
	BOOST_CHECK_EQUAL(12, e.value());
}

BOOST_AUTO_TEST_CASE(expected_move_construct_from_default)
{
	Si::expected<int> e;
	Si::expected<int> f = std::move(e);
	BOOST_CHECK(f.valid());
	BOOST_CHECK_EQUAL(0, f.value());
}

BOOST_AUTO_TEST_CASE(expected_construct_const_value)
{
	Si::expected<int> const e(12);
	BOOST_CHECK(e.valid());
	BOOST_CHECK_EQUAL(12, e.value());
}

BOOST_AUTO_TEST_CASE(expected_construct_inplace)
{
	Si::expected<std::pair<int, float>> e(12, 2.0f);
	BOOST_CHECK(e.valid());
	BOOST_CHECK(std::make_pair(12, 2.0f) == e.value());
}

BOOST_AUTO_TEST_CASE(expected_emplace_0)
{
	Si::expected<int> e;
	BOOST_CHECK(e.valid());
	e.emplace();
	BOOST_CHECK(e.valid());
	BOOST_CHECK_EQUAL(0, e.value());
}

BOOST_AUTO_TEST_CASE(expected_emplace_1)
{
	Si::expected<int> e;
	BOOST_CHECK(e.valid());
	e.emplace(12);
	BOOST_CHECK(e.valid());
	BOOST_CHECK_EQUAL(12, e.value());
	e.emplace(13);
	BOOST_CHECK(e.valid());
	BOOST_CHECK_EQUAL(13, e.value());
}

BOOST_AUTO_TEST_CASE(expected_emplace_2)
{
	Si::expected<std::pair<double, long>> e;
	BOOST_CHECK(e.valid());
	e.emplace(12.0, 4L);
	BOOST_CHECK(e.valid());
	BOOST_CHECK(std::make_pair(12.0, 4L) == e.value());
	e.emplace(13.0, 5L);
	BOOST_CHECK(e.valid());
	BOOST_CHECK(std::make_pair(13.0, 5L) == e.value());
}

namespace
{
	struct test_exception : std::exception
	{
		void *payload;

		explicit test_exception(void *payload)
		    : payload(payload)
		{
		}
	};
}

BOOST_AUTO_TEST_CASE(expected_construct_exception)
{
	int dummy;
	try
	{
		throw boost::enable_current_exception(test_exception(&dummy));
	}
	catch (...)
	{
		Si::expected<int> e((boost::current_exception()));
		BOOST_CHECK(!e.valid());
		BOOST_CHECK_EXCEPTION(e.value(), test_exception,
		                      [&dummy](test_exception const &ex)
		                      {
			                      return (&dummy == ex.payload);
			                  });

		Si::expected<int> moved = std::move(e);
		BOOST_CHECK(!moved.valid());
		BOOST_CHECK(!e.valid());
		BOOST_CHECK_EXCEPTION(moved.value(), test_exception,
		                      [&dummy](test_exception const &ex)
		                      {
			                      return (&dummy == ex.payload);
			                  });

		Si::expected<int> move_assigned;
		move_assigned = std::move(moved);

		BOOST_CHECK(!moved.valid());
		BOOST_CHECK(!move_assigned.valid());
		BOOST_CHECK_EXCEPTION(move_assigned.value(), test_exception,
		                      [&dummy](test_exception const &ex)
		                      {
			                      return (&dummy == ex.payload);
			                  });

		moved = std::move(move_assigned);
		BOOST_CHECK(!moved.valid());
		BOOST_CHECK(!move_assigned.valid());
		BOOST_CHECK_EXCEPTION(moved.value(), test_exception,
		                      [&dummy](test_exception const &ex)
		                      {
			                      return (&dummy == ex.payload);
			                  });
	}
}
#endif
