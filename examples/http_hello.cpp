#include <silicium/http/http.hpp>
#include <silicium/asio/tcp_acceptor.hpp>
#include <silicium/asio/reading_observable.hpp>
#include <silicium/asio/writing_observable.hpp>
#include <silicium/observable/transform.hpp>
#include <silicium/observable/flatten.hpp>
#include <silicium/observable/coroutine.hpp>
#include <silicium/observable/constant.hpp>
#include <silicium/observable/total_consumer.hpp>
#include <silicium/source/error_extracting_source.hpp>
#include <silicium/source/enumerating_source.hpp>
#include <silicium/source/ref.hpp>
#include <silicium/source/observable_source.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/memory_range.hpp>

void serve_client(boost::asio::ip::tcp::socket &client, Si::yield_context yield)
{
	std::array<char, 4096> incoming_buffer;
	auto receiver = Si::make_reading_observable(client, Si::make_memory_range(incoming_buffer));
	auto without_errors = Si::make_error_extracting_source(Si::make_observable_source(std::move(receiver), yield));
	auto enumerated = Si::make_enumerating_source(Si::ref_source(without_errors));
	auto request = Si::http::parse_request(enumerated);
	if (!request)
	{
		//The header was incomplete, maybe the connecting was closed.
		//If we want to know the reason, the error_extracting_source remembered it:
		boost::system::error_code error = without_errors.get_last_error();
		boost::ignore_unused_variable_warning(error);
		return;
	}

	std::vector<char> response;
	{
		auto response_writer = Si::make_container_sink(response);
		Si::http::generate_status_line(response_writer, "HTTP/1.0", "200", "OK");
		std::string const content = "Hello";
		Si::http::generate_header(response_writer, "Content-Length", boost::lexical_cast<Si::noexcept_string>(content.size()));
		Si::append(response_writer, "\r\n");
		Si::append(response_writer, content);
	}

	//you can handle the error if you want
	boost::system::error_code error = Si::write(client, Si::make_memory_range(response), yield);

	//ignore shutdown failures, they do not matter here
	client.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
}

int main()
{
	boost::asio::io_service io;
	boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 8080));
	auto accept_loop = Si::make_total_consumer(Si::flatten(Si::transform(Si::tcp_acceptor(acceptor), [](Si::tcp_acceptor_result maybe_client)
	{
		auto client = maybe_client.get();
		auto client_handler = Si::make_coroutine([client](Si::yield_context yield) -> Si::nothing
		{
			serve_client(*client, yield);
			return {};
		});
		return Si::erase_unique(std::move(client_handler));
	})));
	accept_loop.start();
	io.run();
}
