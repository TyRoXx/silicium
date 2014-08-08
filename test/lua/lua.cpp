#include <reactive/bridge.hpp>
#include <reactive/consume.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
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
	inline void swap_top(lua_State &lua)
	{
		lua_pushvalue(&lua, -2);
		lua_remove(&lua, -3);
	}

	struct pinned_value
	{
		pinned_value() BOOST_NOEXCEPT
		{
		}

		pinned_value(pinned_value &&other) BOOST_NOEXCEPT
			: lua(other.lua)
		{
			*this = std::move(other);
		}

		explicit pinned_value(lua_State &lua, int idx)
			: lua(&lua)
		{
			push_key(*this);
			lua_pushvalue(&lua, idx);
			lua_settable(&lua, LUA_REGISTRYINDEX);
		}

		~pinned_value() BOOST_NOEXCEPT
		{
			if (!lua)
			{
				return;
			}
			remove_this();
		}

		pinned_value &operator = (pinned_value &&other) BOOST_NOEXCEPT
		{
			if (lua)
			{
				remove_this();
			}
			else
			{
				lua = other.lua;
			}
			if (!other.lua)
			{
				lua = nullptr;
				return *this;
			}
			assert(lua == other.lua);

			push_key(other);
			lua_gettable(lua, LUA_REGISTRYINDEX);

			push_key(*this);
			swap_top(*lua);
			lua_settable(lua, LUA_REGISTRYINDEX);
			return *this;
		}

	private:

		lua_State *lua = nullptr;

		BOOST_DELETED_FUNCTION(pinned_value(pinned_value const &))
		BOOST_DELETED_FUNCTION(pinned_value &operator = (pinned_value const &))

		void remove_this()
		{
			push_key(*this);
			lua_pushnil(lua);
			lua_settable(lua, LUA_REGISTRYINDEX);
		}

		void push_key(pinned_value &pin)
		{
			lua_pushlightuserdata(lua, &pin);
		}
	};

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
			int rc;
			if (s->was_resumed)
			{
				rc = lua_resume(s->thread, 0);
			}
			else
			{
				s->was_resumed = true;
				void *bound_state = lua_newuserdata(s->thread, sizeof(weak_state));
				new (static_cast<weak_state *>(bound_state)) weak_state(s);

				//TODO error handling
				//TODO reuse the metatable
				lua_newtable(s->thread);
				lua_pushcfunction(s->thread, garbage_collect_weak_state);
				lua_setfield(s->thread, -2, "__gc");
				lua_setmetatable(s->thread, -2);

				lua_pushcclosure(s->thread, lua_thread::yield, 1);
				rc = lua_resume(s->thread, 1);
			}
			if (LUA_YIELD == rc)
			{
				return;
			}
			if (rc != 0)
			{
				throw std::logic_error("error handling not implemented yet");
			}
			if (s->receiver)
			{
				exchange(s->receiver, nullptr)->ended();
			}
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(s->receiver);
			s.reset();
		}

	private:

		struct state
		{
			lua_State *thread = nullptr;
			ElementFromLua from_lua;
			bool was_resumed = false;
			observer<element_type> *receiver = nullptr;
			pinned_value thread_pin;

			explicit state(lua_State &thread, ElementFromLua from_lua)
				: thread(&thread)
				, from_lua(std::move(from_lua))
			{
				lua_pushthread(&thread);
				thread_pin = pinned_value(thread, -1);
				lua_pop(&thread, 1);
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
			return lua_yield(thread, 0);
		}

		static int garbage_collect_weak_state(lua_State *thread)
		{
			auto &ptr = *static_cast<weak_state *>(lua_touserdata(thread, 1));
			ptr.~weak_state();
			return 0;
		}
	};

	template <class Element, class ElementFromLua>
	auto make_lua_thread(lua_State &parent, ElementFromLua &&from_lua) -> lua_thread<Element, typename std::decay<ElementFromLua>::type>
	{
		lua_State * const coro = lua_newthread(&parent);
		lua_xmove(&parent, coro, 1);
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
		std::vector<lua_Integer> generated;
		auto consumer = rx::consume<lua_Integer>([&generated](boost::optional<lua_Integer> element)
		{
			BOOST_REQUIRE(element);
			generated.emplace_back(*element);
		});
		thread.async_get_one(consumer);
		BOOST_REQUIRE_EQUAL(1, generated.size());

		//make sure that the Lua thread is kept alive properly by trying to collect it before the next resume
		lua_gc(L.get(), LUA_GCCOLLECT, 0);

		thread.async_get_one(consumer);
		BOOST_REQUIRE_EQUAL(2, generated.size());
		std::vector<lua_Integer> const expected{4, 5};
		BOOST_CHECK(expected == generated);
	}
}
