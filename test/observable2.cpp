#include <silicium/observable2.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(observable2_never)
{
	Si::observables2::never<int> n;
	n.observe([](int)
	          {
		          BOOST_FAIL("no element expected");
		      });
}

BOOST_AUTO_TEST_CASE(observable2_transform)
{
	std::pair<Si::observables2::pipe_reader<int>, Si::observables2::pipe_writer<int>> pipe =
	    Si::observables2::create_pipe<int>();
	bool first_callback = false;
	bool second_callback = false;
	auto transformed = Si::observables2::transform(std::move(pipe.first), [&first_callback, &second_callback](int value)
	                                               {
		                                               BOOST_REQUIRE(!first_callback);
		                                               BOOST_REQUIRE(!second_callback);
		                                               first_callback = true;
		                                               return static_cast<long long>(value) * 2;
		                                           });
	BOOST_CHECK(!first_callback);
	BOOST_CHECK(!second_callback);
	transformed.observe([&first_callback, &second_callback](long long value)
	                    {
		                    BOOST_REQUIRE(first_callback);
		                    BOOST_REQUIRE(!second_callback);
		                    second_callback = true;
		                    BOOST_CHECK_EQUAL(6, value);
		                });
	BOOST_CHECK(!first_callback);
	BOOST_CHECK(!second_callback);
	pipe.second.push(3);
	BOOST_CHECK(first_callback);
	BOOST_CHECK(second_callback);
}
