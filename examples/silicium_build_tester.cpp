#include <silicium/async_process.hpp>
#include <silicium/http/generate_response.hpp>
#include <silicium/http/receive_request.hpp>
#include <silicium/asio/tcp_acceptor.hpp>
#include <silicium/asio/writing_observable.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/observable/spawn_coroutine.hpp>
#include <silicium/absolute_path.hpp>
#include <iostream>
#include <array>
#include <boost/program_options.hpp>
#include <boost/asio/spawn.hpp>

namespace
{
	void respond(boost::asio::ip::tcp::socket &client, Si::memory_range status, Si::memory_range status_text, Si::memory_range content, Si::spawn_context &yield)
	{
		std::vector<char> response;
		auto const response_sink = Si::make_container_sink(response);
		Si::http::generate_status_line(response_sink, "HTTP/1.1", status, status_text);
		Si::http::generate_header(response_sink, "Content-Length", boost::lexical_cast<Si::noexcept_string>(content.size()));
		Si::http::generate_header(response_sink, "Content-Type", "text/html");
		Si::http::finish_headers(response_sink);
		Si::append(response_sink, content);
		boost::system::error_code error = Si::asio::write(client, Si::make_memory_range(response), yield);
		if (!!error)
		{
			std::cerr << "Could not respond to " << client.remote_endpoint() << ": " << error << '\n';
			return;
		}
		client.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
		if (!!error)
		{
			std::cerr << "Could not shutdown connection to " << client.remote_endpoint() << ": " << error << '\n';
		}
	}

	bool handle_error(boost::system::error_code error, char const *message)
	{
		if (error)
		{
			std::cerr << message << ": " << error << '\n';
			return true;
		}
		return false;
	}

	enum class output_chunk_result
	{
		more,
		finished
	};

	output_chunk_result handle_output_chunk(Si::native_file_descriptor readable_output)
	{
		std::array<char, 8192> read_buffer;
		ssize_t const read_result = read(readable_output, read_buffer.data(), read_buffer.size());
		if (read_result < 0)
		{
			int errno_ = errno;
			boost::system::error_code error(errno_, boost::system::get_system_category());
			std::cerr << "Reading the output of the process failed with " << error << '\n';
			return output_chunk_result::finished;
		}
		if (read_result == 0)
		{
			//end of output
			return output_chunk_result::finished;
		}
		std::cerr.write(read_buffer.data(), read_result);
		return output_chunk_result::more;
	}

	void clone(
		std::string const &repository,
		Si::absolute_path const &repository_cache)
	{
		Si::async_process_parameters parameters;
		parameters.executable = *Si::absolute_path::create("/usr/bin/git");
		parameters.arguments.emplace_back("clone");
		parameters.arguments.emplace_back(repository.c_str());
		parameters.arguments.emplace_back(repository_cache.c_str());
		parameters.current_path = repository_cache;
		auto output = Si::make_pipe().get();
		auto input = Si::make_pipe().get();
		Si::error_or<Si::async_process> maybe_process = Si::launch_process(parameters, input.read.handle, output.write.handle, output.write.handle);
		if (handle_error(maybe_process.error(), "Could not create git process"))
		{
			return;
		}
		input.read.close();
		output.write.close();
		Si::async_process &process = maybe_process.get();
		for (bool more = true; more;)
		{
			switch (handle_output_chunk(output.read.handle))
			{
			case output_chunk_result::more:
				break;

			case output_chunk_result::finished:
				more = false;
				break;
			}
		}
		int const result = process.wait_for_exit().get();
		if (result != 0)
		{
			std::cerr << "Git clone failed\n";
			return;
		}
	}

	void trigger_build(
		std::string const &repository,
		Si::absolute_path const &repository_cache)
	{
		if (handle_error(Si::create_directories(repository_cache), "Could not create repository cache directory"))
		{
			return;
		}

		if (!Si::file_exists(repository_cache / Si::relative_path(".git")))
		{
			std::cerr << "The cache does not exist. Doing an initial clone of " << repository << "\n";
			clone(repository, repository_cache);
			std::cerr << "Created initial clone of the repository\n";
		}
	}

	void serve_web_client(
		boost::asio::ip::tcp::socket &client,
		std::string const &repository,
		Si::absolute_path const &repository_cache,
		Si::spawn_context &yield)
	{
		Si::error_or<Si::optional<Si::http::request>> const received = Si::http::receive_request(client, yield);
		if (received.is_error())
		{
			std::cerr << "Error when receiving request from " << client.remote_endpoint() << ": " << received.error() << '\n';
			return;
		}

		Si::optional<Si::http::request> const &maybe_request = received.get();
		if (!maybe_request)
		{
			return respond(client, Si::make_c_str_range("400"), Si::make_c_str_range("Bad Request"), Si::make_c_str_range("the server could not parse the request"), yield);
		}

		Si::http::request const &request = *maybe_request;
		if (request.method != "POST" && request.method != "GET")
		{
			return respond(client, Si::make_c_str_range("405"), Si::make_c_str_range("Method Not Allowed"), Si::make_c_str_range("this HTTP method is not supported by this server"), yield);
		}

		if (request.path != "/")
		{
			return respond(client, Si::make_c_str_range("404"), Si::make_c_str_range("Not Found"), Si::make_c_str_range("unknown path"), yield);
		}

		if (request.method == "POST")
		{
			trigger_build(repository, repository_cache);
		}

		char const * const page =
			"<html>"
				"<body>"
					"<form action=\"/\" method=\"POST\">"
						"<input type=\"submit\" value=\"Trigger build\"/>"
					"</form>"
				"</body>"
			"</html>";
		return respond(client, Si::make_c_str_range("200"), Si::make_c_str_range("OK"), Si::make_c_str_range(page), yield);
	}

	void handle_client(
		std::shared_ptr<boost::asio::ip::tcp::socket> client,
		std::string const &repository,
		Si::absolute_path const &repository_cache)
	{
		assert(client);
		Si::spawn_coroutine([client = std::move(client), &repository, &repository_cache](Si::spawn_context yield)
		{
			serve_web_client(*client, repository, repository_cache, yield);
		});
	}

	std::shared_ptr<boost::asio::ip::tcp::socket> async_accept(boost::asio::ip::tcp::acceptor &acceptor, boost::asio::yield_context yield)
	{
		auto client = std::make_shared<boost::asio::ip::tcp::socket>(acceptor.get_io_service());
		acceptor.async_accept(*client, yield);
		return client;
	}
}

int main(int argc, char **argv)
{
	std::string repository;
	std::string cache;
	boost::uint16_t listen_port = 8080;

	boost::program_options::options_description options("options");
	options.add_options()
		("help,h", "produce help message")
		("repository,r", boost::program_options::value(&repository), "the Git URI to clone from")
		("cache,c", boost::program_options::value(&cache), "the directory to use as a cache for the repository")
		("port,p", boost::program_options::value(&listen_port), "the port to listen on for HTTP requests")
		;
	boost::program_options::positional_options_description positional;
	positional.add("repository", 1);
	positional.add("cache", 1);
	boost::program_options::variables_map variables;
	try
	{
		boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(options).positional(positional).run(), variables);
		boost::program_options::notify(variables);
	}
	catch (boost::program_options::error const &ex)
	{
		std::cerr << options << '\n';
		std::cerr << ex.what() << '\n';
		return 1;
	}

	if (variables.count("help"))
	{
		std::cerr << options << '\n';
		return 0;
	}

	if (repository.empty() || cache.empty())
	{
		std::cerr << options << '\n';
		std::cerr << "Missing option\n";
		return 1;
	}

	Si::absolute_path const absolute_cache = *Si::absolute_path::create(boost::filesystem::absolute(cache));

	boost::asio::io_service io;
	boost::asio::spawn(io, [&io, listen_port, &repository, &absolute_cache](boost::asio::yield_context yield)
	{
		boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), listen_port));
		for (;;)
		{
			std::shared_ptr<boost::asio::ip::tcp::socket> client = async_accept(acceptor, yield);
			handle_client(std::move(client), repository, absolute_cache);
		}
	});
	io.run();
}
