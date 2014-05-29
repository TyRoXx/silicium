#include <silicium/oxid.hpp>
#include <silicium/process.hpp>
#include <silicium/tcp_trigger.hpp>
#include <silicium/http/http.hpp>
#include <silicium/asio/socket_sink.hpp>
#include <silicium/asio/socket_source.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/asio/spawn.hpp>
#include <fstream>

namespace
{
	namespace web
	{
		struct acceptor
		{
			explicit acceptor(boost::asio::ip::tcp::acceptor listener)
				: m_listener(std::move(listener))
			{
			}

			template <class RequestHandler>
			void async_accept(RequestHandler handle_request)
			{
				m_client.reset(new boost::asio::ip::tcp::socket(m_listener.get_io_service()));
				m_listener.async_accept(*m_client, [this, handle_request](boost::system::error_code error)
				{
					if (error)
					{
						//TODO
						return;
					}

					//TODO: remove the shared_ptr when we can move the unique_ptr into the callback
					std::shared_ptr<boost::asio::ip::tcp::socket> client(std::move(m_client));

					boost::asio::spawn(m_listener.get_io_service(), [client, handle_request](boost::asio::yield_context yield)
					{
						try
						{
							Si::socket_source request_source(*client, yield);
							Si::socket_sink response_sink(*client, yield);
							handle_request(request_source, response_sink, yield);
						}
						catch (boost::system::system_error const &)
						{
							//socket disconnect
						}
					});
				});
			}

		private:

			std::unique_ptr<boost::asio::ip::tcp::socket> m_client;
			boost::asio::ip::tcp::acceptor m_listener;
		};
	}

	bool run_logging_process(
			std::string executable,
			std::vector<std::string> arguments,
			boost::filesystem::path const &build_dir,
			Si::directory_builder &artifacts,
			std::string const &log_name)
	{
		auto log = artifacts.begin_artifact(log_name);
		auto flushing_log = Si::make_auto_flush_sink(*log);
		return Si::run_process(executable, arguments, build_dir, flushing_log) == 0;
	}

	Si::build_result run_tests(
			boost::filesystem::path const &source,
			boost::filesystem::path const &build_dir,
			Si::directory_builder &artifacts,
			unsigned parallelization)
	{
		if (!run_logging_process("/usr/bin/cmake", {source.string()}, build_dir, artifacts, "cmake.log") != 0)
		{
			return Si::build_failure{"CMake failed"};
		}
		if (!run_logging_process("/usr/bin/make", {"-j" + boost::lexical_cast<std::string>(parallelization)}, build_dir, artifacts, "make.log"))
		{
			return Si::build_failure{"make failed"};
		}
		if (!run_logging_process("test/test", {}, build_dir, artifacts, "test.log"))
		{
			return Si::build_failure{"tests failed"};
		}
		return Si::build_success{};
	}

	std::string format_commit_message(Si::build_result result, git_commit const &source)
	{
		auto const result_str = Si::to_short_string(result);
		auto const author = git_commit_author(&source)->name;
		auto const original_message = git_commit_message(&source);
		return boost::str(boost::format("%1% - %2% - %3%") % result_str % author % original_message);
	}

	struct web_server
	{
		explicit web_server(boost::asio::io_service &io)
			: m_acceptor(boost::asio::ip::tcp::acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 8080)))
		{
		}

		void start()
		{
			m_acceptor.async_accept([this](Si::source<char> &in, Si::sink<char> &out, boost::asio::yield_context &)
			{
				start();

				auto request = Si::http::parse_header(in);
				if (!request)
				{
					//TODO
					return;
				}

				std::string const body = "Hello, world!";

				Si::http::response_header response;
				response.status = 200;
				response.status_text = "OK";
				response.http_version = "HTTP/1.0";
				response.arguments["Content-Length"] = boost::lexical_cast<std::string>(body.size());
				response.arguments["Content-Type"] = "text/html";
				response.arguments["Connection"] = "close";
				write_header(out, response);
				append(out, body);
			});
		}

	private:

		web::acceptor m_acceptor;
	};
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		std::cerr << "At least two arguments required (source path, workspace path)\n";
		return 1;
	}
	boost::filesystem::path const source_location = argv[1];
	boost::filesystem::path const workspace = argv[2];
	unsigned parallelization = 1;
	if (argc >= 4)
	{
		parallelization = boost::lexical_cast<unsigned>(argv[3]);
	}

	auto const log_file = workspace / "error.log";
	std::ofstream log(log_file.string());
	if (!log)
	{
		std::cerr << "Could not open log " << log_file << '\n';
		return 1;
	}

	auto const build = [&]
	{
		try
		{
			auto const run_tests2 = std::bind(run_tests, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, parallelization);
			auto const results_repository = workspace / "results.git";
			Si::oxid::check_build(source_location, results_repository, "master", workspace, "git", format_commit_message, run_tests2);
		}
		catch (std::exception const &ex)
		{
			log << ex.what() << "\n\n";
			log.flush();
		}
	};

	boost::asio::io_service io;
	Si::tcp_trigger external_build_trigger(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 12345));
	std::function<void ()> wait_for_trigger;
	wait_for_trigger = [&wait_for_trigger, &external_build_trigger, &build]
	{
		external_build_trigger.async_wait([&wait_for_trigger, &build]
		{
			build();
			wait_for_trigger();
		});
	};
	wait_for_trigger();

	web_server web(io);
	web.start();

	build();
	io.run();
}
