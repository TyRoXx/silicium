#include <silicium/source_observable.hpp>
#include <silicium/source/memory_source.hpp>
#include <silicium/consume.hpp>
#include <boost/test/unit_test.hpp>

namespace
{
	template <class Element>
	struct end_observer : Si::observer<Element>
	{
		typedef Element element_type;

		bool has_ended;

		end_observer()
			: has_ended(false)
		{
		}

		virtual void got_element(element_type value) SILICIUM_OVERRIDE
		{
			boost::ignore_unused_variable_warning(value);
			BOOST_FAIL("no element expected");
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			BOOST_REQUIRE(!has_ended);
			has_ended = true;
		}
	};
}

BOOST_AUTO_TEST_CASE(source_observable_empty)
{
	std::vector<char> const empty;
	auto observable = Si::make_source_observable<char>(Si::make_container_source(empty));
	end_observer<char> observer;
	observable.async_get_one(observer);
	BOOST_CHECK(observer.has_ended);
}

BOOST_AUTO_TEST_CASE(source_observable_non_empty)
{
	std::string const input = "A";
	auto observable = Si::make_source_observable<char>(Si::make_container_source(input));
	{
		bool got_element = false;
		auto observer = Si::consume<char>([&got_element](char c)
		{
			BOOST_CHECK_EQUAL('A', c);
			BOOST_REQUIRE(!got_element);
			got_element = true;
		});
		observable.async_get_one(observer);
		BOOST_REQUIRE(got_element);
	}
	end_observer<char> observer;
	observable.async_get_one(observer);
	BOOST_CHECK(observer.has_ended);
}
