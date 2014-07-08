#include <reactive/bridge.hpp>
#include <reactive/consume.hpp>
#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>
#include <lua.hpp>

namespace Si
{
	struct lua_deleter
	{
		void operator()(lua_State *L) const
		{
			lua_close(L);
		}
	};

	std::unique_ptr<lua_State, lua_deleter> open_lua()
	{
		auto L = std::unique_ptr<lua_State, lua_deleter>(luaL_newstate());
		if (!L)
		{
			throw std::bad_alloc();
		}
		return L;
	}

	typedef rx::observer<lua_Integer> yield_destination;

	static int yield(lua_State *L)
	{
		yield_destination &dest = *static_cast<yield_destination *>(lua_touserdata(L, lua_upvalueindex(1)));
		lua_Integer element = lua_tointeger(L, 1);
		dest.got_element(element);
		return lua_yield(L, 0);
	}

	BOOST_AUTO_TEST_CASE(lua)
	{
		auto L = open_lua();

		lua_State * const coro = lua_newthread(L.get());
		BOOST_REQUIRE_EQUAL(0, luaL_loadstring(coro, "return function (yield) yield(4) yield(5) end"));
		if (0 != lua_pcall(coro, 0, 1, 0))
		{
			throw std::runtime_error(lua_tostring(L.get(), -1));
		}

		rx::bridge<lua_Integer> yielded;
		lua_pushlightuserdata(coro, &static_cast<yield_destination &>(yielded));
		lua_pushcclosure(coro, yield, 1);

		boost::optional<lua_Integer> got;
		auto consumer = rx::consume<lua_Integer>([&got](boost::optional<lua_Integer> element)
		{
			BOOST_REQUIRE(element);
			got = element;
		});

		{
			yielded.async_get_one(consumer);
			int rc = lua_resume(coro, 1);
			if (LUA_YIELD != rc)
			{
				throw std::runtime_error(lua_tostring(coro, -1));
			}
			BOOST_CHECK_EQUAL(boost::make_optional<lua_Integer>(4), got);
		}

		{
			yielded.async_get_one(consumer);
			int rc = lua_resume(coro, 1);
			if (LUA_YIELD != rc)
			{
				throw std::runtime_error(lua_tostring(coro, -1));
			}
			BOOST_CHECK_EQUAL(boost::make_optional<lua_Integer>(5), got);
		}
	}
}

namespace rx
{
	template <class Element, class ElementFromLua>
	struct lua_thread : public observable<Element>
	{
		using element_type = Element;

		explicit lua_thread(lua_State &thread, ElementFromLua from_lua)
			: s(std::make_shared<state>(thread, std::move(from_lua)))
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			s->receiver = &receiver;
			if (s->was_resumed)
			{
				lua_resume(s->thread, 0);
			}
			else
			{
				void *bound_state = lua_newuserdata(s->thread, sizeof(weak_state));
				new (static_cast<weak_state *>(bound_state)) weak_state(s);
				//TODO __gc
				//TODO error handling
				lua_pushcclosure(s->thread, lua_thread::yield, 1);
				lua_resume(s->thread, 1);
			}
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			s.reset();
		}

	private:

		struct state
		{
			//TODO: keep the Lua thread alive
			lua_State *thread = nullptr;
			ElementFromLua from_lua;
			bool was_resumed = false;
			observer<element_type> *receiver = nullptr;

			explicit state(lua_State &thread, ElementFromLua from_lua)
				: thread(&thread)
				, from_lua(std::move(from_lua))
			{
			}
		};
		typedef std::weak_ptr<state> weak_state;

		std::shared_ptr<state> s;

		static int yield(lua_State *thread)
		{
			weak_state * const bound_state = static_cast<weak_state *>(lua_touserdata(thread, lua_upvalueindex(1)));
			std::shared_ptr<state> locked_state = bound_state->lock();
			if (!locked_state)
			{
				return 0;
			}
			assert(locked_state->receiver);
			exchange(locked_state->receiver, nullptr)->got_element(locked_state->from_lua(*thread, -1));
			return 0;
		}
	};

	inline void swap_top(lua_State &lua)
	{
		lua_pushvalue(&lua, -2);
		lua_remove(&lua, -3);
	}

	template <class Element, class ElementFromLua>
	auto make_lua_thread(lua_State &parent, ElementFromLua &&from_lua)
	{
		lua_State * const coro = lua_newthread(&parent);
		lua_xmove(&parent, coro, 2);
		swap_top(*coro);
		return lua_thread<Element, typename std::decay<ElementFromLua>::type>(parent, std::forward<ElementFromLua>(from_lua));
	}
}

namespace Si
{
	BOOST_AUTO_TEST_CASE(lua_thread_observable)
	{
		auto L = open_lua();
		BOOST_REQUIRE_EQUAL(0, luaL_loadstring(L.get(), "return function (yield) yield(4) yield(5) end"));
		if (0 != lua_pcall(L.get(), 0, 1, 0))
		{
			throw std::runtime_error(lua_tostring(L.get(), -1));
		}

		auto thread = rx::make_lua_thread<lua_Integer>(*L, [](lua_State &lua, int idx) -> lua_Integer { return lua_tointeger(&lua, idx); });
	}
}
