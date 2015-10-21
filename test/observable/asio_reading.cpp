#include <silicium/asio/reading_observable.hpp>
#include <silicium/asio/writing_observable.hpp>
#include <silicium/observable/function_observer.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/array.hpp>
#include <algorithm>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <silicium/pipe.hpp>

#ifndef _WIN32
BOOST_AUTO_TEST_CASE(asio_reading_observable)
{
	boost::asio::io_service io;
	Si::pipe p = Si::make_pipe().move_value();
	boost::asio::posix::stream_descriptor reader(io, p.read.release());
	boost::asio::posix::stream_descriptor writer(io, p.write.release());
	boost::array<char, 1024> read_buffer;
	auto reading_observable = Si::asio::make_reading_observable(reader, Si::make_memory_range(read_buffer));
	std::string received;
	reading_observable.async_get_one(
	    Si::make_function_observer([&](boost::optional<Si::error_or<Si::memory_range>> element)
	                               {
		                               BOOST_REQUIRE(element);
		                               BOOST_REQUIRE(received.empty());
		                               BOOST_REQUIRE(!element->is_error());
		                               BOOST_REQUIRE_EQUAL(5, element->get().size());
		                               received.assign(element->get().begin(), element->get().end());
		                           }));
	bool sent = false;
	writer.async_write_some(boost::asio::buffer("Hello", 5), [&](boost::system::error_code ec, std::size_t written)
	                        {
		                        BOOST_CHECK(!ec);
		                        BOOST_CHECK_EQUAL(5, written);
		                        sent = true;
		                    });
	io.run();
	BOOST_CHECK(sent);
	BOOST_CHECK_EQUAL("Hello", received);
}
#endif
