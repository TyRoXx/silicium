#include <silicium/asio/reading_observable.hpp>
#include <silicium/posix/process.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/array.hpp>

namespace Si
{
	template <class Function>
	struct function_observer
	{
		explicit function_observer(Function function)
			: m_function(std::move(function))
		{
		}

		template <class Element>
		void got_element(Element &&element)
		{
			m_function(std::forward<Element>(element));
		}

		void ended()
		{
			m_function(boost::none);
		}

	private:

		Function m_function;
	};

	template <class Function>
	auto make_function_observer(Function &&function)
	{
		return function_observer<typename std::decay<Function>::type>(std::forward<Function>(function));
	}
}

BOOST_AUTO_TEST_CASE(asio_reading_observable)
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
	writer.async_write_some(boost::asio::buffer("Hello", 5), [&](boost::system::error_code ec, std::size_t written)
	{
		BOOST_CHECK(!ec);
		BOOST_CHECK_EQUAL(5, written);
	});
	io.run();
	BOOST_CHECK_EQUAL("Hello", received);
}
