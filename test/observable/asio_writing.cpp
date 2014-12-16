#include <silicium/asio/reading_observable.hpp>
#include <silicium/asio/writing_observable.hpp>
#include <silicium/posix/process.hpp>
#include <silicium/observable/function_observer.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/array.hpp>

BOOST_AUTO_TEST_CASE(asio_writing_observable)
{
	boost::asio::io_service io;
	Si::detail::pipe p = Si::detail::make_pipe();
	boost::asio::posix::stream_descriptor reader(io, p.read.release());
	boost::asio::posix::stream_descriptor writer(io, p.write.release());
	boost::array<char, 1024> read_buffer;
	auto reading_observable = Si::asio::make_reading_observable(reader, Si::make_memory_range(read_buffer));
	std::string received;
	reading_observable.async_get_one(Si::make_function_observer([&](Si::error_or<Si::memory_range> element)
	{
		BOOST_REQUIRE(received.empty());
		BOOST_REQUIRE(!element.is_error());
		BOOST_REQUIRE_EQUAL(5, element.get().size());
		received.assign(element.get().begin(), element.get().end());
	}));
	auto writing_observable = Si::asio::make_writing_observable(writer);
	writing_observable.set_buffer(Si::make_c_str_range("Hello"));
	bool sent = false;
	writing_observable.async_get_one(Si::make_function_observer([&sent](boost::system::error_code ec)
	{
		BOOST_REQUIRE(!sent);
		BOOST_REQUIRE(!ec);
		sent = true;
	}));
	io.run();
	BOOST_CHECK(sent);
	BOOST_CHECK_EQUAL("Hello", received);
}
