#include <silicium/aligned_ref.hpp>
#include <boost/test/unit_test.hpp>

#if SILICIUM_HAS_ALIGNED_REF
BOOST_AUTO_TEST_CASE(aligned_ref_ok)
{
	std::uint32_t buffer;
	Si::optional<Si::aligned_ref<char, sizeof(buffer)>> a =
	    Si::aligned_ref<char, sizeof(buffer)>::create(reinterpret_cast<char &>(buffer));
	BOOST_REQUIRE(a);
	BOOST_CHECK_EQUAL(reinterpret_cast<char *>(&buffer), &a->ref());
}

BOOST_AUTO_TEST_CASE(aligned_ref_off_by_one)
{
	std::uint32_t buffer;
	BOOST_STATIC_ASSERT(sizeof(buffer) == 4);
	Si::optional<Si::aligned_ref<char, sizeof(buffer)>> a =
	    Si::aligned_ref<char, sizeof(buffer)>::create(reinterpret_cast<char *>(&buffer)[1]);
	BOOST_CHECK(!a);
}

BOOST_AUTO_TEST_CASE(aligned_ref_off_by_two)
{
	std::uint32_t buffer;
	BOOST_STATIC_ASSERT(sizeof(buffer) == 4);
	Si::optional<Si::aligned_ref<char, sizeof(buffer)>> a =
	    Si::aligned_ref<char, sizeof(buffer)>::create(reinterpret_cast<char *>(&buffer)[2]);
	BOOST_CHECK(!a);
}

BOOST_AUTO_TEST_CASE(aligned_ref_off_by_three)
{
	std::uint32_t buffer;
	BOOST_STATIC_ASSERT(sizeof(buffer) == 4);
	Si::optional<Si::aligned_ref<char, sizeof(buffer)>> a =
	    Si::aligned_ref<char, sizeof(buffer)>::create(reinterpret_cast<char *>(&buffer)[3]);
	BOOST_CHECK(!a);
}

BOOST_AUTO_TEST_CASE(aligned_ref_copy)
{
	std::uint32_t const from = 0x12345678;
	std::uint32_t to = 0x00000000;
	Si::copy(*Si::aligned_ref<char const, sizeof(from)>::create(reinterpret_cast<char const &>(from)),
	         *Si::aligned_ref<char, sizeof(to)>::create(reinterpret_cast<char &>(to)), sizeof(from));
	BOOST_CHECK_EQUAL(from, to);
}
#endif
