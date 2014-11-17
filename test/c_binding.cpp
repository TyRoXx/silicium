#include <boost/test/unit_test.hpp>
#include <boost/concept_check.hpp>
#include <c_binding/silicium.h>

BOOST_AUTO_TEST_CASE(c_binding_coroutine_immediate_destruction)
{
	silicium_observable * const coro = silicium_make_coroutine([](void *user_data, silicium_yield_context *yield) -> void *
	{
		boost::ignore_unused_variable_warning(user_data);
		boost::ignore_unused_variable_warning(yield);
		return nullptr;
	}, nullptr);
	BOOST_REQUIRE(coro);
	silicium_free_observable(coro);
}

BOOST_AUTO_TEST_CASE(c_binding_coroutine_trivial)
{
	silicium_observable * const coro = silicium_make_coroutine([](void *user_data, silicium_yield_context *yield) -> void *
	{
		boost::ignore_unused_variable_warning(user_data);
		boost::ignore_unused_variable_warning(yield);
		return reinterpret_cast<void *>(static_cast<std::uintptr_t>(3));
	}, nullptr);
	BOOST_REQUIRE(coro);
	bool got_sth = false;
	silicium_async_get_one(coro, [](void *user_data, void *element)
	{
		BOOST_CHECK_EQUAL(3, reinterpret_cast<std::uintptr_t>(element));
		*static_cast<bool *>(user_data) = true;
	}, &got_sth);
	BOOST_CHECK(got_sth);
	silicium_free_observable(coro);
}
