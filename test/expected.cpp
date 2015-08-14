#include <silicium/expected.hpp>
#include <boost/test/unit_test.hpp>

#if SILICIUM_HAS_EXPECTED
BOOST_AUTO_TEST_CASE(expected_default_construct)
{
	Si::expected<int> e;
	BOOST_CHECK_EQUAL(0, e.value());
}

BOOST_AUTO_TEST_CASE(expected_construct_value)
{
	Si::expected<int> e(12);
	BOOST_CHECK_EQUAL(12, e.value());
}

BOOST_AUTO_TEST_CASE(expected_emplace)
{
	Si::expected<int> e;
	e.emplace(12);
	BOOST_CHECK_EQUAL(12, e.value());
	e.emplace(13);
	BOOST_CHECK_EQUAL(13, e.value());
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
		BOOST_CHECK_EXCEPTION(e.value(), test_exception, [&dummy](test_exception const &ex)
		{
			return (&dummy == ex.payload);
		});
	}
}
#endif
