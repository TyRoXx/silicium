#include <silicium/async_process.hpp>
#include <silicium/http/generate_response.hpp>
#include <silicium/http/receive_request.hpp>
#include <silicium/http/uri.hpp>
#include <silicium/asio/tcp_acceptor.hpp>
#include <silicium/asio/writing_observable.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/absolute_path.hpp>
#include <silicium/asio/socket_source.hpp>
#include <silicium/html.hpp>
#include <silicium/make_array.hpp>
#include <silicium/source/file_source.hpp>
#include <silicium/terminate_on_exception.hpp>
#include <iostream>
#include <array>
#include <boost/program_options.hpp>

#if BOOST_VERSION >= 105400
#include <boost/asio/spawn.hpp>

namespace
{
	template <class YieldContext>
	void respond(boost::asio::ip::tcp::socket &client, Si::memory_range status, Si::memory_range status_text, Si::memory_range content, YieldContext yield)
	{
		boost::system::error_code error;
		auto const remote_endpoint = client.remote_endpoint(error);
		if (!!error)
		{
			std::cerr << "Client disconnected\n";
			return;
		}

		std::vector<char> response;
		auto const response_sink = Si::make_container_sink(response);
		Si::http::generate_status_line(response_sink, "HTTP/1.1", status, status_text);
		Si::http::generate_header(response_sink, "Content-Length", boost::lexical_cast<Si::noexcept_string>(content.size()));
		Si::http::generate_header(response_sink, "Content-Type", "text/html");
		Si::http::finish_headers(response_sink);

		auto const write_data = Si::make_array<boost::asio::const_buffer>(
			boost::asio::buffer(response),
			boost::asio::buffer(content.begin(), content.size())
		);

		boost::asio::async_write(client, write_data, yield[error]);
		if (!!error)
		{
			std::cerr << "Could not respond to " << remote_endpoint << ": " << error << '\n';
			return;
		}
		client.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
		if (!!error)
		{
			std::cerr << "Could not shutdown connection to " << remote_endpoint << ": " << error << '\n';
		}
	}

	SILICIUM_USE_RESULT
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

	SILICIUM_USE_RESULT
	output_chunk_result handle_output_chunk(Si::native_file_descriptor readable_output)
	{
		std::array<char, 8192> read_buffer;
		Si::error_or<std::size_t> const read_result = Si::read(readable_output, Si::make_memory_range(read_buffer));
		if (read_result.is_error())
		{
			std::cerr << "Reading the output of the process failed with " << read_result.error() << '\n';
			return output_chunk_result::finished;
		}
		if (read_result.get() == 0)
		{
			//end of output
			return output_chunk_result::finished;
		}
		assert(read_result.get() <= read_buffer.size());
		std::cerr.write(read_buffer.data(), read_result.get());
		return output_chunk_result::more;
	}

	Si::absolute_path const git_exe = *Si::absolute_path::create(
#ifdef _WIN32
		L"C:\\Program Files (x86)\\Git\\bin\\git.exe"
#else
		"/usr/bin/git"
#endif
		);

	Si::absolute_path const cmake_exe = *Si::absolute_path::create(
#ifdef _WIN32
		L"C:\\Program Files (x86)\\CMake\\bin\\cmake.exe"
#else
		"/usr/bin/cmake"
#endif
		);

	SILICIUM_USE_RESULT
	Si::optional<int> execute_process(Si::absolute_path executable, std::vector<Si::os_string> arguments, Si::absolute_path working_directory)
	{
		Si::async_process_parameters parameters;
		parameters.executable = executable;
		parameters.current_path = working_directory;
		parameters.arguments = std::move(arguments);
		auto output = SILICIUM_MOVE_IF_COMPILER_LACKS_RVALUE_QUALIFIERS(Si::make_pipe().get());
		auto input = SILICIUM_MOVE_IF_COMPILER_LACKS_RVALUE_QUALIFIERS(Si::make_pipe().get());
		Si::error_or<Si::async_process> maybe_process = Si::launch_process(parameters, input.read.handle, output.write.handle, output.write.handle);
		if (handle_error(maybe_process.error(), "Could not create process"))
		{
			return Si::none;
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
		return result;
	}

	enum class success_or_failure
	{
		success,
		failure
	};

	success_or_failure clone(
		Si::os_string const &repository,
		Si::absolute_path const &repository_cache)
	{
		std::vector<Si::os_string> arguments;
		arguments.emplace_back(Si::to_os_string("clone"));
		arguments.emplace_back(repository);
		arguments.emplace_back(repository_cache.c_str());
		Si::optional<int> const result = execute_process(git_exe, std::move(arguments), repository_cache);
		if (!result)
		{
			return success_or_failure::failure;
		}
		if (*result != 0)
		{
			std::cerr << "Git clone failed with " << *result << "\n";
			return success_or_failure::failure;
		}
		return success_or_failure::success;
	}

	SILICIUM_USE_RESULT success_or_failure fetch(Si::absolute_path const &repository)
	{
		std::vector<Si::os_string> arguments;
		arguments.emplace_back(Si::to_os_string("fetch"));
		Si::optional<int> const result = execute_process(git_exe, std::move(arguments), repository);
		if (!result)
		{
			return success_or_failure::failure;
		}
		if (*result != 0)
		{
			std::cerr << "Git fetch failed with " << *result << "\n";
			return success_or_failure::failure;
		}
		return success_or_failure::success;
	}
	
	SILICIUM_USE_RESULT success_or_failure update_repository(
		Si::os_string const &origin,
		Si::absolute_path const &repository_cache)
	{
		if (handle_error(Si::create_directories(repository_cache), "Could not create repository cache directory"))
		{
			return success_or_failure::failure;
		}

		auto const cache_git_dir = repository_cache / Si::relative_path(".git");
		Si::error_or<bool> const cached = Si::file_exists(cache_git_dir);
		if (cached.is_error())
		{
			std::cerr << "Could not determine whether " << cache_git_dir << " exists\n";
			return success_or_failure::failure;
		}

		if (cached.get())
		{
			std::cerr << "Fetching the cached repository " << Si::to_utf8_string(origin) << "\n";
			switch (fetch(repository_cache))
			{
			case success_or_failure::success:
				std::cerr << "Fetched the repository\n";
				break;

			case success_or_failure::failure:
				return success_or_failure::failure;
			}
		}
		else
		{
			std::cerr << "The cache does not exist. Doing an initial clone of " << Si::to_utf8_string(origin) << "\n";
			switch (clone(origin, repository_cache))
			{
			case success_or_failure::success:
				std::cerr << "Created initial clone of the repository\n";
				break;

			case success_or_failure::failure:
				return success_or_failure::failure;
			}
		}
		return success_or_failure::success;
	}

	SILICIUM_USE_RESULT success_or_failure checkout(Si::absolute_path const &repository)
	{
		std::vector<Si::os_string> arguments;
		arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("checkout"));
		arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("origin/master"));
		Si::optional<int> const result = execute_process(git_exe, std::move(arguments), repository);
		if (!result)
		{
			std::cerr << "Could not start Git process " << git_exe << '\n';
			return success_or_failure::failure;
		}
		if (*result != 0)
		{
			std::cerr << "Git checkout failed with " << *result << "\n";
			return success_or_failure::failure;
		}
		return success_or_failure::success;
	}

	struct build_state
	{
		Si::optional<std::future<void>> current_build_process;
	};

	template <class Body, class Finalizer>
	void finally(Body &&body, Finalizer &&finalize)
	{
		try
		{
			std::forward<Body>(body)();
		}
		catch (...)
		{
			std::forward<Finalizer>(finalize)();
			throw;
		}
		std::forward<Finalizer>(finalize)();
	}

	SILICIUM_USE_RESULT
	success_or_failure cmake_generate(
		Si::absolute_path const &source,
		Si::absolute_path const &build)
	{
		std::vector<Si::os_string> arguments;
		arguments.emplace_back(source.c_str());
		Si::optional<int> const result = execute_process(cmake_exe, std::move(arguments), build);
		if (!result)
		{
			std::cerr << "Could not start CMake process " << cmake_exe << '\n';
			return success_or_failure::failure;
		}
		if (*result != 0)
		{
			std::cerr << "CMake failed with " << *result << "\n";
			return success_or_failure::failure;
		}
		return success_or_failure::success;
	}

	SILICIUM_USE_RESULT
	success_or_failure cmake_build(Si::absolute_path const &build)
	{
		std::vector<Si::os_string> arguments;
		arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("--build"));
		arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("."));
#ifndef _WIN32
		arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("--"));
		arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("-j12"));
#endif
		Si::optional<int> const result = execute_process(cmake_exe, std::move(arguments), build);
		if (!result)
		{
			std::cerr << "Could not start CMake process " << cmake_exe << '\n';
			return success_or_failure::failure;
		}
		if (*result != 0)
		{
			std::cerr << "CMake build failed with " << *result << "\n";
			return success_or_failure::failure;
		}
		return success_or_failure::success;
	}

	SILICIUM_USE_RESULT
	success_or_failure run_silicium_tests(Si::absolute_path const &build)
	{
		Si::absolute_path const test_dir = build / Si::relative_path("test");
		Si::absolute_path const test_exe = test_dir / Si::relative_path("unit_test");
		std::vector<Si::os_string> arguments;
		arguments.emplace_back(SILICIUM_SYSTEM_LITERAL("--progress=yes"));
		Si::optional<int> const result = execute_process(test_exe, std::move(arguments), test_dir);
		if (!result)
		{
			std::cerr << "Could not start unit test process process " << test_exe << '\n';
			return success_or_failure::failure;
		}
		if (*result != 0)
		{
			std::cerr << "Tests failed with " << *result << "\n";
			return success_or_failure::failure;
		}
		return success_or_failure::success;
	}

	void build_silicium(
		Si::os_string const &origin,
		Si::absolute_path const &workspace)
	{
		Si::absolute_path const repository_cache = workspace / Si::relative_path("cache.git");
		switch (update_repository(origin, repository_cache))
		{
		case success_or_failure::success:
			break;

		case success_or_failure::failure:
			std::cerr << "Could not update the repository\n";
			return;
		}

		switch (checkout(repository_cache))
		{
		case success_or_failure::success:
			break;

		case success_or_failure::failure:
			std::cerr << "Git checkout failed\n";
			return;
		}

		Si::absolute_path const build_directory = workspace / Si::relative_path("build");
		if (handle_error(Si::recreate_directories(build_directory), "Could not clear the build directory"))
		{
			return;
		}

		switch (cmake_generate(repository_cache, build_directory))
		{
		case success_or_failure::success:
			break;

		case success_or_failure::failure:
			std::cerr << "CMake generate failed\n";
			return;
		}

		switch (cmake_build(build_directory))
		{
		case success_or_failure::success:
			break;

		case success_or_failure::failure:
			std::cerr << "CMake build failed\n";
			return;
		}

		switch (run_silicium_tests(build_directory))
		{
		case success_or_failure::success:
			break;

		case success_or_failure::failure:
			std::cerr << "Silicium tests failed\n";
			return;
		}
	}

	template <class Action>
	auto async_call(boost::asio::io_service &io, Action &&action)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> std::future<decltype(std::forward<Action>(action)())>
#endif
	{
		auto keep_io_running = std::make_shared<boost::asio::io_service::work>(io);
		return std::async(std::launch::async, [
			SILICIUM_CAPTURE_EXPRESSION(action, std::forward<Action>(action)),
			SILICIUM_CAPTURE_EXPRESSION(keep_io_running, std::move(keep_io_running))]() mutable
		{
			return std::forward<Action>(action)();
		});
	}

	void trigger_build(
		boost::asio::io_service &synchronizer,
		build_state &state,
		Si::os_string const &origin,
		Si::absolute_path const &workspace)
	{
		if (state.current_build_process)
		{
			std::cerr << "We are already building\n";
			return;
		}
		std::cerr << "Starting build thread\n";
		state.current_build_process = async_call(
			synchronizer,
			[origin, workspace, &synchronizer, &state]
		{
			finally(
				[&origin, &workspace]()
				{
					build_silicium(origin, workspace);
				},
				[&state, &synchronizer]
				{
					synchronizer.post([&state]
					{
						std::cerr << "Build thread finished\n";
						std::future<void> result = std::move(*state.current_build_process);
						state.current_build_process = Si::none;
						try
						{
							result.get();
						}
						catch (std::exception const &ex)
						{
							std::cerr << "The build thread failed with an exception: " << ex.what() << '\n';
						}
					});
				}
			);
		});
	}

	template <class YieldContext>
	void serve_web_client(
		boost::asio::ip::tcp::socket &client,
		Si::os_string const &repository,
		Si::absolute_path const &workspace,
		build_state &state,
		YieldContext yield)
	{
		Si::asio::basic_socket_source<YieldContext> receiver(client, yield);
		Si::optional<Si::http::request> maybe_request;
		boost::system::error_code error;
		auto const remote_endpoint = client.remote_endpoint(error);
		if (!!error)
		{
			std::cerr << "Client disconnected\n";
			return;
		}

		//TODO: do this without an exception
		try
		{
			//TODO: virtualize_source should not be necessary here
			maybe_request = Si::http::parse_request(Si::virtualize_source(Si::ref_source(receiver)));
		}
		catch (boost::system::system_error const &ex)
		{
			std::cerr << "Error when receiving request from " << remote_endpoint << ": " << ex.code() << '\n';
			return;
		}

		if (!maybe_request)
		{
			return respond(client, Si::make_c_str_range("400"), Si::make_c_str_range("Bad Request"), Si::make_c_str_range("the server could not parse the request"), yield);
		}

		Si::http::request const &request = *maybe_request;
		if (request.method != "POST" && request.method != "GET")
		{
			return respond(client, Si::make_c_str_range("405"), Si::make_c_str_range("Method Not Allowed"), Si::make_c_str_range("this HTTP method is not supported by this server"), yield);
		}

		Si::optional<Si::http::uri> const maybe_parsed_path = Si::http::parse_uri(Si::make_contiguous_range(request.path));
		if (!maybe_parsed_path)
		{
			return respond(client, Si::make_c_str_range("400"), Si::make_c_str_range("Bad Request"), Si::make_c_str_range("the requested path has an unknown format"), yield);
		}
		Si::http::uri const &parsed_path = *maybe_parsed_path;

		if (!parsed_path.path.empty())
		{
			return respond(client, Si::make_c_str_range("404"), Si::make_c_str_range("Not Found"), Si::make_c_str_range("unknown path"), yield);
		}

		std::vector<char> page;
		auto html = Si::html::make_generator(Si::make_container_sink(page));
		html("html", [&]
		{
			html("head", [&]
			{
				html("title", [&]
				{
					html.write("Silicium build tester");
				});
			});
			html("body", [&]
			{
				if (request.method == "POST")
				{
					trigger_build(client.get_io_service(), state, repository, workspace);
					html.write("build was triggered");
				}
				html("form",
					[&]
				{
					html.attribute("action", "/");
					html.attribute("method", "POST");
				},
					[&]
				{
					html("input",
						[&]
					{
						html.attribute("type", "submit");
						html.attribute("value", "Trigger build");
					},
						[&]
					{
					});
				});
			});

		});
		return respond(client, Si::make_c_str_range("200"), Si::make_c_str_range("OK"), Si::make_memory_range(page), yield);
	}

	void handle_client(
		std::shared_ptr<boost::asio::ip::tcp::socket> client,
		Si::os_string const &repository,
		Si::absolute_path const &workspace,
		build_state &state)
	{
		assert(client);
		auto on_exit = []()
		{
		};
		boost::asio::spawn(
			on_exit,
			[
				SILICIUM_CAPTURE_EXPRESSION(client, std::move(client)),
				&repository,
				&workspace,
				&state
			](boost::asio::basic_yield_context<decltype(on_exit) &> yield)
			{
				serve_web_client(*client, repository, workspace, state, yield);
			}
		);
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
	std::string workspace;
	boost::uint16_t listen_port = 8080;

	boost::program_options::options_description options("options");
	options.add_options()
		("help,h", "produce help message")
		("repository,r", boost::program_options::value(&repository), "the Git URI to clone from")
		("workspace,w", boost::program_options::value(&workspace), "the directory for temporary files (git cache, build output)")
		("port,p", boost::program_options::value(&listen_port), "the port to listen on for HTTP requests")
		;
	boost::program_options::positional_options_description positional;
	positional.add("repository", 1);
	positional.add("workspace", 1);
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

	if (repository.empty() || workspace.empty())
	{
		std::cerr << options << '\n';
		std::cerr << "Missing option\n";
		return 1;
	}

	try
	{
		Si::absolute_path const absolute_workspace = *Si::absolute_path::create(boost::filesystem::absolute(workspace));
		Si::os_string const repository_as_os_string = Si::to_os_string(repository);

		boost::asio::io_service io;
		build_state state;

		boost::asio::spawn(io, [&io, listen_port, &repository_as_os_string, &absolute_workspace, &state](boost::asio::yield_context yield)
		{
			boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), listen_port));
			for (;;)
			{
				std::shared_ptr<boost::asio::ip::tcp::socket> client = async_accept(acceptor, yield);
				handle_client(std::move(client), repository_as_os_string, absolute_workspace, state);
			}
		});
		io.run();
	}
	catch (std::exception const &ex)
	{
		std::cerr << ex.what() << '\n';
		return 1;
	}
}
#else
int main()
{
	std::cerr << "This example requires boost::asio::spawn (Boost 1.54+)\n";
}
#endif
