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

BOOST_AUTO_TEST_CASE(serialization_le_uint32_parser_one_byte_at_a_time)
{
	Si::serialization::le_uint32_parser p;
	BOOST_REQUIRE(nullptr == p.check_result());
	std::array<Si::serialization::byte, sizeof(boost::uint32_t)> const input =
	{{
		0x01, 0xEF, 0xCD, 0xAB
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

BOOST_AUTO_TEST_CASE(serialization_le_uint32_parser_all_at_once)
{
	Si::serialization::le_uint32_parser p;
	BOOST_REQUIRE(nullptr == p.check_result());
	std::array<Si::serialization::byte, sizeof(boost::uint32_t)> const input =
	{{
			0x01, 0xEF, 0xCD, 0xAB
	}};
	BOOST_REQUIRE(input.end() == p.consume_input(input.begin(), input.end()));
	BOOST_REQUIRE(nullptr != p.check_result());
	BOOST_CHECK_EQUAL(0xABCDEF01U, *p.check_result());
}

BOOST_AUTO_TEST_CASE(serialization_le_uint32_parser_all_at_once_with_rest)
{
	Si::serialization::le_uint32_parser p;
	BOOST_REQUIRE(nullptr == p.check_result());
	std::array<Si::serialization::byte, sizeof(boost::uint32_t) + 1> const input =
	{{
			0x01, 0xEF, 0xCD, 0xAB, 0x44
	}};
	BOOST_REQUIRE((input.begin() + sizeof(boost::uint32_t)) == p.consume_input(input.begin(), input.begin() + sizeof(boost::uint32_t)));
	BOOST_REQUIRE(nullptr != p.check_result());
	BOOST_CHECK_EQUAL(0xABCDEF01U, *p.check_result());
}
