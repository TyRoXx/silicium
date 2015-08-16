#include <silicium/safe_arithmetic.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(safe_arithmetic_add_size_t)
{
	Si::safe_number<std::size_t> a(12), b(54);
	Si::overflow_or<Si::safe_number<std::size_t>> result = a + b;
	BOOST_REQUIRE(!result.is_overflow());
	BOOST_CHECK_EQUAL(66, result.value()->value);
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_add_size_t_overflow)
{
	Si::safe_number<std::size_t> a((std::numeric_limits<std::size_t>::max)() - 1), b(2);
	Si::overflow_or<Si::safe_number<std::size_t>> result = a + b;
	BOOST_CHECK(result.is_overflow());
}

namespace
{
	template <class Unsigned>
	void test_add_overloads()
	{
		typedef Si::overflow_or<Si::safe_number<Unsigned>> overflow_uint;

		BOOST_CHECK_EQUAL(Si::safe<Unsigned>(5), Si::safe<Unsigned>(2) + Si::safe<Unsigned>(3));
		BOOST_CHECK_EQUAL(Si::overflow, Si::safe((std::numeric_limits<Unsigned>::max)()) + Si::safe<Unsigned>(3));
		BOOST_CHECK_EQUAL(Si::overflow, Si::safe<Unsigned>(3) + Si::safe((std::numeric_limits<Unsigned>::max)()));
		BOOST_CHECK_EQUAL(Si::overflow, Si::safe<Unsigned>(3) + Si::overflow);
		BOOST_CHECK_EQUAL(Si::overflow, Si::overflow + Si::safe<Unsigned>(3));

		BOOST_CHECK_EQUAL(Si::safe<Unsigned>(5), Si::safe<Unsigned>(2) + overflow_uint(Si::safe<Unsigned>(3)));
		BOOST_CHECK_EQUAL(Si::overflow, Si::safe((std::numeric_limits<Unsigned>::max)()) + overflow_uint(Si::safe<Unsigned>(3)));
		BOOST_CHECK_EQUAL(Si::overflow, Si::safe<Unsigned>(3) + overflow_uint(Si::safe((std::numeric_limits<Unsigned>::max)())));
		BOOST_CHECK_EQUAL(Si::overflow, Si::safe<Unsigned>(3) + overflow_uint(Si::overflow));
		BOOST_CHECK_EQUAL(Si::overflow, Si::overflow + overflow_uint(Si::safe<Unsigned>(3)));

		BOOST_CHECK_EQUAL(Si::safe<Unsigned>(5), overflow_uint(Si::safe<Unsigned>(2)) + Si::safe<Unsigned>(3));
		BOOST_CHECK_EQUAL(Si::overflow, overflow_uint(Si::safe((std::numeric_limits<Unsigned>::max)())) + Si::safe<Unsigned>(3));
		BOOST_CHECK_EQUAL(Si::overflow, overflow_uint(Si::safe<Unsigned>(3)) + Si::safe((std::numeric_limits<Unsigned>::max)()));
		BOOST_CHECK_EQUAL(Si::overflow, overflow_uint(Si::safe<Unsigned>(3)) + Si::overflow);
		BOOST_CHECK_EQUAL(Si::overflow, overflow_uint(Si::overflow) + Si::safe<Unsigned>(3));

		BOOST_CHECK_EQUAL(overflow_uint(Si::safe<Unsigned>(5)), Si::safe<Unsigned>(2) + Si::safe<Unsigned>(3));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), Si::safe((std::numeric_limits<Unsigned>::max)()) + Si::safe<Unsigned>(3));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), Si::safe<Unsigned>(3) + Si::safe((std::numeric_limits<Unsigned>::max)()));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), Si::safe<Unsigned>(3) + Si::overflow);
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), Si::overflow + Si::safe<Unsigned>(3));

		BOOST_CHECK_EQUAL(overflow_uint(Si::safe<Unsigned>(5)), overflow_uint(Si::safe<Unsigned>(2)) + overflow_uint(Si::safe<Unsigned>(3)));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), overflow_uint(Si::safe((std::numeric_limits<Unsigned>::max)())) + overflow_uint(Si::safe<Unsigned>(3)));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), overflow_uint(Si::safe<Unsigned>(3)) + overflow_uint(Si::safe((std::numeric_limits<Unsigned>::max)())));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), overflow_uint(Si::safe<Unsigned>(3)) + overflow_uint(Si::overflow));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), overflow_uint(Si::overflow) + overflow_uint(Si::safe<Unsigned>(3)));
	}

	template <class Unsigned>
	void test_mul_overloads()
	{
		typedef Si::overflow_or<Si::safe_number<Unsigned>> overflow_uint;

		BOOST_CHECK_EQUAL(Si::safe<Unsigned>(6), Si::safe<Unsigned>(2) * Si::safe<Unsigned>(3));
		BOOST_CHECK_EQUAL(Si::overflow, Si::safe((std::numeric_limits<Unsigned>::max)()) * Si::safe<Unsigned>(3));
		BOOST_CHECK_EQUAL(Si::overflow, Si::safe<Unsigned>(3) * Si::safe((std::numeric_limits<Unsigned>::max)()));
		BOOST_CHECK_EQUAL(Si::overflow, Si::safe<Unsigned>(3) * Si::overflow);
		BOOST_CHECK_EQUAL(Si::overflow, Si::overflow * Si::safe<Unsigned>(3));

		BOOST_CHECK_EQUAL(Si::safe<Unsigned>(6), Si::safe<Unsigned>(2) * overflow_uint(Si::safe<Unsigned>(3)));
		BOOST_CHECK_EQUAL(Si::overflow, Si::safe((std::numeric_limits<Unsigned>::max)()) * overflow_uint(Si::safe<Unsigned>(3)));
		BOOST_CHECK_EQUAL(Si::overflow, Si::safe<Unsigned>(3) * overflow_uint(Si::safe((std::numeric_limits<Unsigned>::max)())));
		BOOST_CHECK_EQUAL(Si::overflow, Si::safe<Unsigned>(3) * overflow_uint(Si::overflow));
		BOOST_CHECK_EQUAL(Si::overflow, Si::overflow * overflow_uint(Si::safe<Unsigned>(3)));

		BOOST_CHECK_EQUAL(Si::safe<Unsigned>(6), overflow_uint(Si::safe<Unsigned>(2)) * Si::safe<Unsigned>(3));
		BOOST_CHECK_EQUAL(Si::overflow, overflow_uint(Si::safe((std::numeric_limits<Unsigned>::max)())) * Si::safe<Unsigned>(3));
		BOOST_CHECK_EQUAL(Si::overflow, overflow_uint(Si::safe<Unsigned>(3)) * Si::safe((std::numeric_limits<Unsigned>::max)()));
		BOOST_CHECK_EQUAL(Si::overflow, overflow_uint(Si::safe<Unsigned>(3)) * Si::overflow);
		BOOST_CHECK_EQUAL(Si::overflow, overflow_uint(Si::overflow) * Si::safe<Unsigned>(3));

		BOOST_CHECK_EQUAL(overflow_uint(Si::safe<Unsigned>(6)), Si::safe<Unsigned>(2) * Si::safe<Unsigned>(3));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), Si::safe((std::numeric_limits<Unsigned>::max)()) * Si::safe<Unsigned>(3));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), Si::safe<Unsigned>(3) * Si::safe((std::numeric_limits<Unsigned>::max)()));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), Si::safe<Unsigned>(3) * Si::overflow);
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), Si::overflow * Si::safe<Unsigned>(3));

		BOOST_CHECK_EQUAL(overflow_uint(Si::safe<Unsigned>(6)), overflow_uint(Si::safe<Unsigned>(2)) * overflow_uint(Si::safe<Unsigned>(3)));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), overflow_uint(Si::safe((std::numeric_limits<Unsigned>::max)())) * overflow_uint(Si::safe<Unsigned>(3)));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), overflow_uint(Si::safe<Unsigned>(3)) * overflow_uint(Si::safe((std::numeric_limits<Unsigned>::max)())));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), overflow_uint(Si::safe<Unsigned>(3)) * overflow_uint(Si::overflow));
		BOOST_CHECK_EQUAL(overflow_uint(Si::overflow), overflow_uint(Si::overflow) * overflow_uint(Si::safe<Unsigned>(3)));
	}

	template <class Unsigned>
	void test_arithmetic_overloads()
	{
		test_add_overloads<Unsigned>();
		test_mul_overloads<Unsigned>();
	}
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_unsigned_char_all_overloads)
{
	test_arithmetic_overloads<unsigned char>();
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_unsigned_short_all_overloads)
{
	test_arithmetic_overloads<unsigned short>();
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_unsigned_all_overloads)
{
	test_arithmetic_overloads<unsigned>();
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_unsigned_long_all_overloads)
{
	test_arithmetic_overloads<unsigned long>();
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_unsigned_long_long_all_overloads)
{
	test_arithmetic_overloads<unsigned long long>();
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_size_t_all_overloads)
{
	test_arithmetic_overloads<std::size_t>();
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_sub_size_t)
{
	Si::safe_number<std::size_t> a(54), b(12);
	Si::overflow_or<Si::safe_number<std::size_t>> result = a - b;
	BOOST_REQUIRE(!result.is_overflow());
	BOOST_CHECK_EQUAL(42, result.value()->value);
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_sub_size_t_overflow)
{
	Si::safe_number<std::size_t> a(1), b(2);
	Si::overflow_or<Si::safe_number<std::size_t>> result = a - b;
	BOOST_CHECK(result.is_overflow());
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_mul_size_t)
{
	Si::safe_number<std::size_t> a(3), b(5);
	Si::overflow_or<Si::safe_number<std::size_t>> result = a * b;
	BOOST_REQUIRE(!result.is_overflow());
	BOOST_CHECK_EQUAL(15, result.value()->value);
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_mul_size_t_overflow)
{
	Si::safe_number<std::size_t> a((std::numeric_limits<std::size_t>::max)() - 1), b(2);
	Si::overflow_or<Si::safe_number<std::size_t>> result = a * b;
	BOOST_CHECK(result.is_overflow());
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_div_size_t)
{
	Si::safe_number<std::size_t> a(50), b(3);
	Si::overflow_or<Si::safe_number<std::size_t>> result = a / b;
	BOOST_REQUIRE(!result.is_overflow());
	BOOST_CHECK_EQUAL(16, result.value()->value);
}

BOOST_AUTO_TEST_CASE(safe_arithmetic_div_size_t_zero)
{
	Si::safe_number<std::size_t> a(23), b(0);
	Si::overflow_or<Si::safe_number<std::size_t>> result = a / b;
	BOOST_CHECK(result.is_overflow());
}
