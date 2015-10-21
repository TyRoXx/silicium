#include <silicium/make_destructor.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
	bool stateless_called_once = false;
}

BOOST_AUTO_TEST_CASE(make_destructor_stateless)
{
	BOOST_REQUIRE(!stateless_called_once);
	{
		auto d = Si::make_destructor([]()
		                             {
			                             BOOST_REQUIRE(!stateless_called_once);
			                             stateless_called_once = true;
			                         });
		BOOST_REQUIRE(!stateless_called_once);
	}
	BOOST_CHECK(stateless_called_once);
}

BOOST_AUTO_TEST_CASE(make_destructor_copyable_state)
{
	bool called_once = false;
	BOOST_REQUIRE(!called_once);
	{
		auto d = Si::make_destructor([&called_once]()
		                             {
			                             BOOST_REQUIRE(!called_once);
			                             called_once = true;
			                         });
		BOOST_REQUIRE(!called_once);
	}
	BOOST_CHECK(called_once);
}

#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
BOOST_AUTO_TEST_CASE(make_destructor_movable_only_state)
{
	bool called_once = false;
	BOOST_REQUIRE(!called_once);
	{
		auto state = Si::make_unique<int>(2);
		auto d = Si::make_destructor([&called_once, state = std::move(state) ]()
		                             {
			                             BOOST_REQUIRE(!called_once);
			                             called_once = true;
			                         });
		BOOST_REQUIRE(!called_once);
	}
	BOOST_CHECK(called_once);
}
#endif
