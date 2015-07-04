#include <boost/test/unit_test.hpp>
#include <boost/concept_check.hpp>
#include <boost/cstdint.hpp>
#include <silicium/config.hpp>
#include <silicium/observable/coroutine.hpp>
#include <c_binding/silicium.h>

BOOST_AUTO_TEST_CASE(c_binding_coroutine_immediate_destruction)
{
	silicium_observable * const coro = silicium_make_coroutine([](void *user_data, silicium_yield_context *yield) -> void *
	{
		boost::ignore_unused_variable_warning(user_data);
		boost::ignore_unused_variable_warning(yield);
		return nullptr;
	}, nullptr);
#if SILICIUM_HAS_COROUTINE_OBSERVABLE
	BOOST_REQUIRE(coro);
	silicium_free_observable(coro);
#else
	BOOST_CHECK(!coro);
#endif
}

BOOST_AUTO_TEST_CASE(c_binding_coroutine_trivial)
{
	silicium_observable * const coro = silicium_make_coroutine([](void *user_data, silicium_yield_context *yield) -> void *
	{
		boost::ignore_unused_variable_warning(user_data);
		boost::ignore_unused_variable_warning(yield);
		return reinterpret_cast<void *>(static_cast<Si::uintptr_t>(3));
	}, nullptr);
#if SILICIUM_HAS_COROUTINE_OBSERVABLE
	BOOST_REQUIRE(coro);
	bool got_sth = false;
	silicium_async_get_one(coro, [](void *user_data, void *element)
	{
		BOOST_CHECK_EQUAL(3u, reinterpret_cast<Si::uintptr_t>(element));
		*static_cast<bool *>(user_data) = true;
	}, &got_sth);
	BOOST_CHECK(got_sth);
	silicium_free_observable(coro);
#else
	BOOST_CHECK(!coro);
#endif
}

BOOST_AUTO_TEST_CASE(c_binding_coroutine_yield)
{
	silicium_observable * const a = silicium_make_coroutine([](void *user_data, silicium_yield_context *yield) -> void *
	{
		boost::ignore_unused_variable_warning(user_data);
		boost::ignore_unused_variable_warning(yield);
		return reinterpret_cast<void *>(static_cast<Si::uintptr_t>(3));
	}, nullptr);
	silicium_observable * const b = silicium_make_coroutine([](void *user_data, silicium_yield_context *yield) -> void *
	{
		auto * const from = static_cast<silicium_observable *>(user_data);
		Si::uintptr_t const got = reinterpret_cast<Si::uintptr_t>(silicium_yield_get_one(yield, from));
		return reinterpret_cast<void *>(static_cast<Si::uintptr_t>(got + 13));
	}, a);
#if SILICIUM_HAS_COROUTINE_OBSERVABLE
	BOOST_REQUIRE(a);
	BOOST_REQUIRE(b);
	bool got_sth = false;
	silicium_async_get_one(b, [](void *user_data, void *element)
	{
		BOOST_CHECK_EQUAL(16u, reinterpret_cast<Si::uintptr_t>(element));
		*static_cast<bool *>(user_data) = true;
	}, &got_sth);
	BOOST_CHECK(got_sth);
	silicium_free_observable(b);
	silicium_free_observable(a);
#else
	BOOST_CHECK(!a);
	BOOST_CHECK(!b);
#endif
}
