#include <silicium/git/oxid.hpp>
#include <silicium/process.hpp>
#include <silicium/sink/throwing_sink.hpp>
#include <silicium/asio/tcp_trigger.hpp>
#include <silicium/http/http.hpp>
#include <silicium/asio/socket_sink.hpp>
#include <silicium/asio/socket_source.hpp>
#include <silicium/asio/accepting_source.hpp>
#include <silicium/source/buffering_source.hpp>
#include <silicium/sink/ptr_sink.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/asio/spawn.hpp>
#include <fstream>
#include <thread>

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
				auto &listener = m_listener;
				m_listener.async_accept(*m_client, [this, &listener, handle_request](boost::system::error_code error)
				{
					if (error)
					{
						//TODO
						return;
					}

					//TODO: remove the shared_ptr when we can move the unique_ptr into the callback
					std::shared_ptr<boost::asio::ip::tcp::socket> client(std::move(m_client));

					auto &io = listener.get_io_service();
					boost::asio::spawn(io, [client, handle_request](boost::asio::yield_context yield)
					{
						try
						{
							Si::asio::socket_source request_source(*client, yield);
							Si::asio::socket_sink response_sink(*client, yield);
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

		namespace html
		{
			template <class OutputIterator>
			void write(char const *c_str, OutputIterator out)
			{
				std::copy(c_str, c_str + std::strlen(c_str), out);
			}

			template <class Char, class OutputIterator>
			void write_char(Char in, OutputIterator out)
			{
				switch (in)
				{
				case '&': write("&amp;", out); break;
				case '<': write("&lt;", out); break;
				case '>': write("&gt;", out); break;
				case '\'': write("&quot;", out); break;
				case '"': write("&apos;", out); break;
				default:
					*out = in;
					++out;
					break;
				}
			}

			template <class OutputIterator>
			void write_string(
					std::string const &text,
					OutputIterator out)
			{
				for (auto c : text)
				{
					write_char(c, out);
				}
			}

			template <class OutputIterator>
			void open_attributed_element(
					std::string const &name,
					OutputIterator out)
			{
				*out++ = '<';
				boost::range::copy(name, out);
			}

			template <class OutputIterator>
			void finish_attributes(OutputIterator out)
			{
				*out++ = '>';
			}

			template <class OutputIterator>
			void add_attribute(
					std::string const &key,
					std::string const &value,
					OutputIterator out)
			{
				boost::range::copy(key, out);
				*out++ = '=';
				*out++ = '"';
				write_escaped(value, out);
				*out++ = '"';
			}

			template <class OutputIterator>
			void open_element(
					std::string const &name,
					OutputIterator out)
			{
				open_attributed_element(name, out);
				finish_attributes(out);
			}

			template <class OutputIterator>
			void close_element(
					std::string const &name,
					OutputIterator out)
			{
				*out++ = '<';
				*out++ = '/';
				boost::range::copy(name, out);
				*out++ = '>';
			}

			template <class OutputIterator>
			struct generator
			{
				generator()
				{
				}

				explicit generator(OutputIterator out)
					: m_out(out)
				{
				}

				template <class ContentMaker>
				void element(std::string const &name, ContentMaker make_content) const
				{
					open_element(name, m_out);
					make_content();
					close_element(name, m_out);
				}

				void write(std::string const &text) const
				{
					write_string(text, m_out);
				}

			private:

				OutputIterator m_out;
			};

			template <class OutputIterator>
			auto make_generator(OutputIterator out)
				-> generator<typename std::decay<OutputIterator>::type>
			{
				return generator<typename std::decay<OutputIterator>::type>(out);
			}
		}
	}

	bool run_logging_process(
			std::string executable,
			std::vector<std::string> arguments,
			boost::filesystem::path const &build_dir,
			Si::directory_builder &artifacts,
			std::string const &log_name)
	{
		auto log = artifacts.begin_artifact(log_name);
		auto throwing_log = Si::make_throwing_sink(Si::ref_sink(*log));
		return Si::run_process(executable, arguments, build_dir, throwing_log) == 0;
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
		typedef std::function<void (Si::source<char> &, Si::sink<char> &, boost::asio::yield_context &)> request_responder;

		explicit web_server(boost::asio::io_service &io, request_responder respond)
			: m_acceptor(boost::asio::ip::tcp::acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 8080)))
			, m_respond(std::move(respond))
		{
		}

		void start()
		{
			m_acceptor.async_accept([this](Si::source<char> &in, Si::sink<char> &out, boost::asio::yield_context &yield)
			{
				start();
				m_respond(in, out, yield);
			});
		}

	private:

		web::acceptor m_acceptor;
		request_responder m_respond;
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

	std::weak_ptr<boost::posix_time::ptime const> current_build;

	boost::asio::io_service io;

	boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 12345));
	boost::asio::spawn(io, [&acceptor, &current_build, &build](boost::asio::yield_context yield)
	{
		Si::asio::accepting_source clients(acceptor, yield);
		for (auto client : (clients | Si::buffered(1)))
		{
			(void)client;
			auto new_build = std::make_shared<boost::posix_time::ptime const>(boost::posix_time::microsec_clock::local_time());
			std::thread([new_build, &build]
			{
				build();
			}).detach();
			current_build = new_build;
		}
	});

	auto const respond_web_request = [&current_build](Si::source<char> &in, Si::sink<char> &out, boost::asio::yield_context &)
	{
		Si::buffering_source<char> buffered_in(in, 1U << 14U);
		auto request = Si::http::parse_request(buffered_in);
		if (!request)
		{
			//TODO
			return;
		}

		std::string body;
		{
			auto page = web::html::make_generator(std::back_inserter(body));
			page.element("html", [&]
			{
				page.element("head", [&]
				{
					page.element("title", [&]
					{
						page.write("silicium");
					});
				});
				page.element("body", [&]
				{
					page.element("h1", [&]
					{
						page.write("Hello, world!");
					});
					page.element("p", [&]
					{
						auto const current = current_build.lock();
						if (current)
						{
							page.write("build running since " + boost::lexical_cast<std::string>(*current));
						}
						else
						{
							page.write("no active build");
						}
					});
				});
			});
		}

		Si::http::response response;
		response.arguments = Si::make_unique<std::map<Si::noexcept_string, Si::noexcept_string>>();
		response.status = 200;
		response.status_text = "OK";
		response.http_version = "HTTP/1.0";
		(*response.arguments)["Content-Length"] = boost::lexical_cast<Si::noexcept_string>(body.size());
		(*response.arguments)["Content-Type"] = "text/html";
		(*response.arguments)["Connection"] = "close";

		auto buffered_out = make_buffering_sink(ref_sink(out));
		write_header(buffered_out, response);
		append(buffered_out, body);
		buffered_out.flush();
	};

	web_server web(io, respond_web_request);
	web.start();

	build();
	io.run();
}
