#include <silicium/bounded_int.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

BOOST_AUTO_TEST_CASE(bounded_int_always_zero)
{
	Si::optional<Si::bounded_int<int, 0, 0>> i =
	    Si::bounded_int<int, 0, 0>::create(0);
	BOOST_REQUIRE(i);
	BOOST_CHECK_EQUAL(0, i->value());
}

BOOST_AUTO_TEST_CASE(bounded_int_out_of_range)
{
	Si::optional<Si::bounded_int<int, 0, 1>> i =
	    Si::bounded_int<int, 0, 1>::create(2);
	BOOST_CHECK(!i);
}

BOOST_AUTO_TEST_CASE(bounded_int_less)
{
	Si::bounded_int<int, 0, 1> zero = Si::bounded_int<int, 0, 1>::literal<0>();
	Si::bounded_int<int, 0, 1> one = Si::bounded_int<int, 0, 1>::literal<1>();
	BOOST_CHECK_LT(zero, one);
	BOOST_CHECK(!(one < zero));
}

BOOST_AUTO_TEST_CASE(bounded_int_format_char)
{
	typedef Si::bounded_int<char,
#if SILICIUM_COMPILER_HAS_CONSTEXPR_NUMERIC_LIMITS
	                        (std::numeric_limits<char>::min)(),
	                        (std::numeric_limits<char>::max)()
#else
	                        CHAR_MIN, CHAR_MAX
#endif
	                        > char_int;
#if SILICIUM_COMPILER_HAS_CONSTEXPR_NUMERIC_LIMITS
	BOOST_STATIC_ASSERT((std::numeric_limits<int>::max)() >
	                    (std::numeric_limits<char>::max)());
#endif
	for (int i = (std::numeric_limits<char>::min)();
	     i <= (std::numeric_limits<char>::max)(); ++i)
	{
		Si::optional<char_int> z = char_int::create(static_cast<char>(i));
		BOOST_REQUIRE(z);
		BOOST_CHECK_EQUAL(boost::lexical_cast<std::string>(static_cast<int>(i)),
		                  boost::lexical_cast<std::string>(z));
	}
}

BOOST_AUTO_TEST_CASE(bounded_int_format_short)
{
	Si::bounded_int<short, 0, 133> zero =
	    Si::bounded_int<short, 0, 133>::literal<133>();
	BOOST_CHECK_EQUAL("133", boost::lexical_cast<std::string>(zero));
}

BOOST_AUTO_TEST_CASE(bounded_int_construct_from_smaller)
{
	Si::bounded_int<int, 0, 1> smaller =
	    Si::bounded_int<int, 0, 1>::literal<1>();
	Si::bounded_int<int, -1, 2> larger = smaller;
	BOOST_CHECK_EQUAL(larger, smaller);
	BOOST_CHECK_EQUAL(1, larger.value());
}

BOOST_AUTO_TEST_CASE(bounded_int_add)
{
	Si::bounded_int<int, 0, 1> first = Si::bounded_int<int, 0, 1>::literal<1>();
	Si::bounded_int<int, -1, 2> second =
	    Si::bounded_int<int, -1, 2>::literal<2>();
	Si::bounded_int<int, -1, 3> result = first + second;
	BOOST_CHECK_EQUAL(3, result.value());
}

BOOST_AUTO_TEST_CASE(bounded_int_clamp_smaller_max)
{
	Si::bounded_int<int, 0, 1> in = Si::bounded_int<int, 0, 1>::literal<1>();
	Si::bounded_int<int, 0, 0> out = in.clamp<0, 0>();
	BOOST_CHECK_EQUAL(0, out.value());
}

BOOST_AUTO_TEST_CASE(bounded_int_clamp_greater_min)
{
	Si::bounded_int<int, 0, 1> in = Si::bounded_int<int, 0, 1>::literal<0>();
	Si::bounded_int<int, 1, 1> out = in.clamp<1, 1>();
	BOOST_CHECK_EQUAL(1, out.value());
}

BOOST_AUTO_TEST_CASE(bounded_int_clamp_value_unchanged)
{
	Si::bounded_int<int, 0, 2> in = Si::bounded_int<int, 0, 2>::literal<1>();
	Si::bounded_int<int, 1, 2> out = in.clamp<1, 2>();
	BOOST_CHECK_EQUAL(1, out.value());
}

BOOST_AUTO_TEST_CASE(bounded_int_narrow_success)
{
	Si::bounded_int<int, 0, 2> in = Si::bounded_int<int, 0, 2>::literal<1>();
	Si::optional<Si::bounded_int<int, 1, 2>> out = in.narrow<1, 2>();
	BOOST_CHECK_EQUAL((Si::bounded_int<int, 1, 2>::literal<1>()), out);
}

BOOST_AUTO_TEST_CASE(bounded_int_narrow_failure)
{
	Si::bounded_int<int, 0, 2> in = Si::bounded_int<int, 0, 2>::literal<0>();
	Si::optional<Si::bounded_int<int, 1, 2>> out = in.narrow<1, 2>();
	BOOST_CHECK_EQUAL(Si::none, out);
}
