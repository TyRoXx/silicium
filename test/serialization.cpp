#include <silicium/serialization.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(serialization_be_uint32_parser)
{
	Si::serialization::be_uint32_parser p;
	BOOST_REQUIRE(nullptr == p.check_result());
	std::array<Si::serialization::byte, sizeof(boost::uint32_t)> const input =
	{{
		0xAB, 0xCD, 0xEF, 0x01
	}};
	BOOST_REQUIRE((input.begin() + 1) == p.consume_input(input.begin(), input.begin() + 1));
	BOOST_REQUIRE(nullptr == p.check_result());
	BOOST_REQUIRE((input.begin() + 2) == p.consume_input(input.begin() + 1, input.begin() + 2));
	BOOST_REQUIRE(nullptr == p.check_result());
	BOOST_REQUIRE((input.begin() + 3) == p.consume_input(input.begin() + 2, input.begin() + 3));
	BOOST_REQUIRE(nullptr == p.check_result());
	BOOST_REQUIRE(input.end() == p.consume_input(input.begin() + 3, input.begin() + 4));
	BOOST_REQUIRE(nullptr != p.check_result());
	BOOST_CHECK_EQUAL(0xABCDEF01U, *p.check_result());
}
