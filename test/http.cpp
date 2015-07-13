#include <silicium/http/http.hpp>
#include <silicium/http/request_parser_sink.hpp>
#include <silicium/source/memory_source.hpp>
#include <silicium/fast_variant.hpp>
#include <silicium/sink/iterator_sink.hpp>
#include <silicium/sink/ptr_sink.hpp>
#include <silicium/observable/function_observer.hpp>
#include <silicium/observable/ref.hpp>
#include <silicium/observable/bridge.hpp>
#include <silicium/observable/consume.hpp>
#include <silicium/error_or.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	BOOST_AUTO_TEST_CASE(http_parse_header)
	{
		std::string const incoming =
				"GET / HTTP/1.0\r\n"
				"Key: Value\r\n"
				"\r\n"
				;
		auto source = Si::make_container_source(incoming);
		Si::optional<Si::http::request> const parsed = Si::http::parse_request(source);
		BOOST_REQUIRE(parsed);
		BOOST_CHECK_EQUAL("GET", parsed->method);
		BOOST_CHECK_EQUAL("/", parsed->path);
		BOOST_CHECK_EQUAL("HTTP/1.0", parsed->http_version);
		std::map<noexcept_string, noexcept_string> const expected_arguments
		{
			{"Key", "Value"}
		};
		BOOST_CHECK(expected_arguments == parsed->arguments);
	}

	BOOST_AUTO_TEST_CASE(http_write_request_header)
	{
		std::string generated;
		auto sink = Si::make_container_sink(generated);
		http::request header;
		header.http_version = "HTTP/1.1";
		header.method = "POST";
		header.path = "/p";
		header.arguments["Content-Length"] = "13";
		http::generate_request(sink, header);
		BOOST_CHECK_EQUAL(
			"POST /p HTTP/1.1\r\n"
			"Content-Length: 13\r\n"
			"\r\n",
			generated);
	}

	BOOST_AUTO_TEST_CASE(response_header_move)
	{
		http::response a;
		http::response b(std::move(a));
		a = std::move(b);
	}

	BOOST_AUTO_TEST_CASE(response_header_copy)
	{
		http::response a;
		http::response b(a);
		a = b;
	}

	BOOST_AUTO_TEST_CASE(response_header_compatible_with_variant)
	{
		variant<http::response> v;
		auto w = v;
		w = std::move(v);
		auto u = std::move(w);
		u = v;
	}

	BOOST_AUTO_TEST_CASE(http_request_parser_sink)
	{
		std::vector<Si::http::request> results;
		auto parser = Si::http::make_request_parser_sink(Si::make_container_sink(results));
		Si::append(parser, "GET / HTTP/1.1\r\n");
		BOOST_CHECK(results.empty());
		Si::append(parser, "Host: host\r\n");
		BOOST_CHECK(results.empty());
		Si::append(parser, "\r\n");
		BOOST_REQUIRE_EQUAL(1u, results.size());
		Si::http::request &result = results[0];
		BOOST_CHECK_EQUAL("GET", result.method);
		BOOST_CHECK_EQUAL("/", result.path);
		BOOST_CHECK_EQUAL("HTTP/1.1", result.http_version);
		BOOST_CHECK_EQUAL(1u, result.arguments.size());
		BOOST_CHECK((std::map<Si::noexcept_string, Si::noexcept_string>{{"Host", "host"}}) == result.arguments);
	}
}

namespace Si
{
	namespace http
	{
		template <class ErrorOrMemoryRangeObservable>
		struct request_parser_observable : private Sink<request, success>::interface, private observer<error_or<memory_range>>
		{
			typedef error_or<request> element_type;

			explicit request_parser_observable(ErrorOrMemoryRangeObservable input)
				: m_input(std::move(input))
				, m_observer(nullptr)
				, m_got_result(false)
			{
			}

			void async_get_one(ptr_observer<observer<element_type>> observer_)
			{
				assert(!m_got_result);
				assert(!m_observer);
				m_observer = observer_.get();
				fetch();
			}

		private:

			ErrorOrMemoryRangeObservable m_input;
			boost::optional<request_parser_sink<ptr_sink<sink<request, success>, sink<request, success> *>>> m_state;
			observer<element_type> *m_observer;
			bool m_got_result;

			void fetch()
			{
				m_input.async_get_one(Si::observe_by_ref(static_cast<observer<error_or<memory_range>> &>(*this)));
			}

			virtual void got_element(error_or<memory_range> piece) SILICIUM_OVERRIDE
			{
				if (piece.is_error())
				{
					Si::exchange(m_observer, nullptr)->got_element(piece.error());
					return;
				}
				if (!m_state)
				{
					m_state = boost::in_place(ref_sink(static_cast<sink<request, success> &>(*this)));
				}
				m_state->append(piece.get());
				if (m_got_result)
				{
					m_got_result = false;
					return;
				}
				fetch();
			}

			virtual void ended() SILICIUM_OVERRIDE
			{
				SILICIUM_UNREACHABLE();
			}

			virtual success append(iterator_range<request const *> data) SILICIUM_OVERRIDE
			{
				assert(data.size() == 1);
				assert(!m_got_result);
				Si::exchange(m_observer, nullptr)->got_element(data.front());
				m_got_result = true;
				return {};
			}
		};

		template <class ErrorOrMemoryRangeObservable>
		auto make_request_parser_observable(ErrorOrMemoryRangeObservable &&input)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			-> request_parser_observable<typename std::decay<ErrorOrMemoryRangeObservable>::type>
#endif
		{
			return request_parser_observable<typename std::decay<ErrorOrMemoryRangeObservable>::type>(std::forward<ErrorOrMemoryRangeObservable>(input));
		}
	}
}

BOOST_AUTO_TEST_CASE(http_parser_observable)
{
	Si::bridge<Si::error_or<Si::memory_range>> input;
	auto parser = Si::http::make_request_parser_observable(Si::ref(input));
	bool got_result = false;
	std::function<void ()> get;
	auto consumer = Si::consume<Si::error_or<Si::http::request>>([&](Si::error_or<Si::http::request> const &result)
	{
		BOOST_REQUIRE(!got_result);
		BOOST_REQUIRE(!result.is_error());
		BOOST_CHECK_EQUAL("GET", result.get().method);
		BOOST_CHECK_EQUAL("/", result.get().path);
		BOOST_CHECK_EQUAL("HTTP/1.0", result.get().http_version);
		BOOST_CHECK(result.get().arguments.empty());
		got_result = true;
		get();
	});
	BOOST_REQUIRE(!got_result);
	get = [&]()
	{
		parser.async_get_one(Si::observe_by_ref(consumer));
	};
	get();
	for (size_t i = 0; i < 10; ++i)
	{
		std::string const request = "GET / HTTP/1.0\r\n\r";
		for (char c : request)
		{
			BOOST_REQUIRE(!got_result);
			input.got_element(Si::make_memory_range(&c, &c + 1));
		}
		BOOST_REQUIRE(got_result);
		got_result = false;
		input.got_element(Si::make_c_str_range("\n"));
		BOOST_REQUIRE(!got_result);
	}
}
