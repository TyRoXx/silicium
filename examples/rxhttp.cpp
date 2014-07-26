#include <reactive/observable_source.hpp>
#include <reactive/tcp_acceptor.hpp>
#include <reactive/coroutine.hpp>
#include <silicium/http/http.hpp>
#include <silicium/fast_variant.hpp>
#include <reactive/finite_state_machine.hpp>
#include <reactive/generate.hpp>
#include <reactive/total_consumer.hpp>
#include <boost/asio.hpp>
#include <boost/ref.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/format.hpp>
#include <boost/interprocess/sync/null_mutex.hpp>
#include <boost/thread/future.hpp>

namespace rx
{
	struct incoming_bytes
	{
		char const *begin = nullptr, *end = nullptr;

		incoming_bytes() BOOST_NOEXCEPT
		{
		}

		incoming_bytes(char const *begin, char const *end) BOOST_NOEXCEPT
			: begin(begin)
			, end(end)
		{
		}
	};

	typedef Si::fast_variant<incoming_bytes, boost::system::error_code> received_from_socket;

	struct socket_observable : observable<received_from_socket>
	{
		typedef received_from_socket element_type;
		typedef boost::iterator_range<char *> buffer_type;

		socket_observable(boost::asio::ip::tcp::socket &socket, buffer_type buffer)
			: socket(&socket)
			, buffer(buffer)
		{
			assert(!buffer.empty());
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			receiver_ = &receiver;
			socket->async_receive(boost::asio::buffer(buffer.begin(), buffer.size()), [this](boost::system::error_code error, std::size_t bytes_received)
			{
				assert(receiver_);
				if (error)
				{
					if (error == boost::asio::error::operation_aborted)
					{
						return;
					}
					exchange(this->receiver_, nullptr)->got_element(error);
				}
				else
				{
					auto * const receiver = exchange(this->receiver_, nullptr);
					receiver->got_element(incoming_bytes{buffer.begin(), buffer.begin() + bytes_received});
				}
			});
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			socket->cancel();
			receiver_ = nullptr;
		}

	private:

		boost::asio::ip::tcp::socket *socket;
		buffer_type buffer;
		observer<element_type> *receiver_ = nullptr;
	};

	struct socket_receiver_observable : observable<char>, private observer<received_from_socket>
	{
		typedef char element_type;
		typedef boost::iterator_range<char *> buffer_type;

		explicit socket_receiver_observable(boost::asio::ip::tcp::socket &socket, buffer_type buffer)
			: socket(&socket)
			, buffer(buffer)
			, next_byte(buffer.begin())
			, available(buffer.begin())
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			if (next_byte != buffer.end())
			{
				char value = *next_byte;
				++next_byte;
				return receiver.got_element(value);
			}
			receiving = boost::in_place(boost::ref(*socket), buffer);
			receiver_ = &receiver;
			receiving->async_get_one(*this);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			throw std::logic_error("to do");
		}

	private:

		boost::asio::ip::tcp::socket *socket;
		buffer_type buffer;
		char const *next_byte;
		char const *available;
		observer<element_type> *receiver_ = nullptr;
		boost::optional<socket_observable> receiving;

		struct received_from_handler : boost::static_visitor<>
		{
			socket_receiver_observable *receiver;

			explicit received_from_handler(socket_receiver_observable &receiver)
				: receiver(&receiver)
			{
			}

			void operator()(boost::system::error_code) const
			{
				//error means end
				assert(receiver->receiver_);
				return rx::exchange(receiver->receiver_, nullptr)->ended();
			}

			void operator()(incoming_bytes bytes_received) const
			{
				assert(std::distance(bytes_received.begin, bytes_received.end) > 0);
				assert(std::distance(bytes_received.begin, bytes_received.end) <= receiver->buffer.size());
				receiver->available = bytes_received.end;
				receiver->next_byte = bytes_received.begin;
				char value = *receiver->next_byte;
				++(receiver->next_byte);
				return rx::exchange(receiver->receiver_, nullptr)->got_element(value);
			}
		};

		virtual void got_element(received_from_socket value) SILICIUM_OVERRIDE
		{
			received_from_handler handler{*this};
			return Si::apply_visitor(handler, value);
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			throw std::logic_error("should not be called");
		}
	};

	template <class T, class ...Args>
	auto make_unique(Args &&...args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}

	using nothing = detail::nothing;

	template <class NothingObservableObservable, class Mutex = boost::interprocess::null_mutex>
	struct flattener
			: public observable<nothing>
			, private observer<typename NothingObservableObservable::element_type>
	{
		explicit flattener(NothingObservableObservable input)
			: input(std::move(input))
			, children_mutex(make_unique<Mutex>())
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			receiver_ = &receiver;
			fetch();
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			throw std::logic_error("to do");
		}

	private:

		typedef typename NothingObservableObservable::element_type nothing_observable;

		struct child : observer<nothing>
		{
			flattener &parent;
			nothing_observable observed;

			explicit child(flattener &parent, nothing_observable observed)
				: parent(parent)
				, observed(observed)
			{
			}

			void start()
			{
				observed.async_get_one(*this);
			}

			virtual void got_element(nothing) SILICIUM_OVERRIDE
			{
				return start();
			}

			virtual void ended() SILICIUM_OVERRIDE
			{
				return parent.remove_child(*this);
			}
		};

		NothingObservableObservable input;
		bool input_ended = false;
		observer<nothing> *receiver_ = nullptr;
		std::vector<std::unique_ptr<child>> children;
		std::unique_ptr<Mutex> children_mutex;

		void fetch()
		{
			return input.async_get_one(*this);
		}

		void remove_child(child &removing)
		{
			boost::unique_lock<Mutex> lock(*children_mutex);
			auto const i = boost::range::find_if(children, [&removing](std::unique_ptr<child> const &element)
			{
				return element.get() == &removing;
			});
			children.erase(i);
			if (input_ended &&
			    children.empty())
			{
				exchange(receiver_, nullptr)->ended();
			}
		}

		virtual void got_element(nothing_observable value) SILICIUM_OVERRIDE
		{
			{
				boost::unique_lock<Mutex> lock(*children_mutex);
				children.emplace_back(make_unique<child>(*this, std::move(value)));
				child &new_child = *children.back();
				new_child.start();
			}
			return fetch();
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			boost::unique_lock<Mutex> lock(*children_mutex);
			assert(receiver_);
			input_ended = true;
			if (children.empty())
			{
				exchange(receiver_, nullptr)->ended();
			}
		}
	};

	template <class Mutex = boost::interprocess::null_mutex, class NothingObservableObservable>
	auto flatten(NothingObservableObservable &&input)
	{
		return flattener<typename std::decay<NothingObservableObservable>::type, Mutex>(std::forward<NothingObservableObservable>(input));
	}

	struct sending_observable : observable<boost::system::error_code>
	{
		typedef boost::system::error_code element_type;
		typedef boost::iterator_range<char const *> buffer_type;

		explicit sending_observable(boost::asio::ip::tcp::socket &socket, buffer_type buffer)
			: socket(&socket)
			, buffer(buffer)
		{
		}

		virtual void async_get_one(observer<boost::system::error_code> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			if (buffer.empty())
			{
				return receiver.ended();
			}
			receiver_ = &receiver;
			boost::asio::async_write(*socket, boost::asio::buffer(buffer.begin(), buffer.size()), [this](boost::system::error_code error, std::size_t bytes_sent)
			{
				assert(buffer.size() == static_cast<ptrdiff_t>(bytes_sent));
				buffer = buffer_type();
				return exchange(receiver_, nullptr)->got_element(error);
			});
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			throw std::logic_error("to do");
		}

	private:

		boost::asio::ip::tcp::socket *socket = nullptr;
		buffer_type buffer;
		observer<boost::system::error_code> *receiver_ = nullptr;
	};
}

namespace Si
{
	namespace detail
	{
		struct error_stripper : boost::static_visitor<boost::optional<rx::incoming_bytes>>
		{
			boost::optional<rx::incoming_bytes> operator()(rx::incoming_bytes bytes) const
			{
				return bytes;
			}

			boost::optional<rx::incoming_bytes> operator()(boost::system::error_code) const
			{
				return boost::none;
			}
		};

		boost::optional<rx::incoming_bytes> strip_error(rx::received_from_socket received)
		{
			return Si::apply_visitor(error_stripper{}, received);
		}
	}

	struct received_from_socket_source : Si::source<char>
	{
		explicit received_from_socket_source(Si::source<rx::received_from_socket> &original)
			: original(&original)
		{
		}

		virtual boost::iterator_range<char const *> map_next(std::size_t) SILICIUM_OVERRIDE
		{
			assert(original);
			if (rest.begin == rest.end)
			{
				auto next = Si::get(*original);
				if (!next)
				{
					return {};
				}
				auto bytes = detail::strip_error(*next);
				if (!bytes)
				{
					return {};
				}
				rest = *bytes;
			}
			return boost::iterator_range<char const *>(rest.begin, rest.end);
		}

		virtual char *copy_next(boost::iterator_range<char *> destination) SILICIUM_OVERRIDE
		{
			auto mapped = map_next(destination.size());
			char * const copied = std::copy_n(mapped.begin(), std::min(destination.size(), mapped.size()), destination.begin());
			skip(std::distance(destination.begin(), copied));
			return copied;
		}

		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE
		{
			return std::distance(rest.begin, rest.end);
		}

		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE
		{
			return boost::none;
		}

		virtual std::size_t skip(std::size_t count) SILICIUM_OVERRIDE
		{
			std::size_t skipped = 0;
			auto const rest_size = std::distance(rest.begin, rest.end);
			auto const rest_skipped = std::min<ptrdiff_t>(count, rest_size);
			rest.begin += rest_skipped;
			count -= rest_skipped;
			skipped += rest_skipped;
			if (count > 0)
			{
				throw std::logic_error("to do");
			}
			return skipped;
		}

	private:

		Si::source<rx::received_from_socket> *original = nullptr;
		rx::incoming_bytes rest{};
	};
}

namespace
{
	void serve_client(boost::asio::ip::tcp::socket &client, rx::yield_context<rx::detail::nothing> &yield, boost::uintmax_t visitor_number)
	{
		std::vector<char> received(4096);
		rx::socket_observable receiving(client, boost::make_iterator_range(received.data(), received.data() + received.size()));
		auto receiver = rx::make_observable_source(rx::ref(receiving), yield);
		Si::received_from_socket_source bytes_receiver(receiver);
		boost::optional<Si::http::request_header> request = Si::http::parse_header(bytes_receiver);
		if (!request)
		{
			return;
		}
		std::vector<char> send_buffer;
		{
			auto response_sink = Si::make_container_sink(send_buffer);
			Si::http::response_header response;
			response.http_version = "HTTP/1.1";
			response.status = 200;
			response.status_text = "OK";
			std::string const body = boost::str(boost::format("Hello, visitor %1%") % visitor_number);
			response.arguments["Content-Length"] = boost::lexical_cast<std::string>(body.size());
			response.arguments["Connection"] = "close";
			Si::http::write_header(response_sink, response);
			Si::append(response_sink, body);
		}
		rx::sending_observable sending(
			client,
			boost::make_iterator_range(send_buffer.data(), send_buffer.data() + send_buffer.size()));

		//ignore error
		yield.get_one(sending);

		boost::system::error_code error;
		client.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);

		while (Si::get(bytes_receiver))
		{
		}
	}

	using events = rx::shared_observable<rx::detail::nothing>;

	struct accept_handler : boost::static_visitor<events>
	{
		boost::uintmax_t visitor_number;

		explicit accept_handler(boost::uintmax_t visitor_number)
			: visitor_number(visitor_number)
		{
		}

		events operator()(std::shared_ptr<boost::asio::ip::tcp::socket> client) const
		{
			auto visitor_number_ = visitor_number;
			auto client_handler = rx::wrap<rx::detail::nothing>(rx::make_coroutine<rx::detail::nothing>([client, visitor_number_](rx::yield_context<rx::detail::nothing> &yield) -> void
			{
				return serve_client(*client, yield, visitor_number_);
			}));
			return client_handler;
		}

		events operator()(boost::system::error_code) const
		{
			throw std::logic_error("not implemented");
		}
	};
}

int main()
{
	boost::asio::io_service io;
	boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 8080));
	rx::tcp_acceptor clients(acceptor);
	auto handling_clients = rx::flatten<boost::mutex>(rx::make_coroutine<events>([&clients](rx::yield_context<events> &yield) -> void
	{
		auto visitor_counter = rx::make_finite_state_machine(
									rx::generate([]() { return rx::nothing{}; }),
									static_cast<boost::uintmax_t>(0),
									[](boost::uintmax_t previous, rx::nothing) { return previous + 1; });
		for (;;)
		{
			auto result = yield.get_one(clients);
			if (!result)
			{
				break;
			}
			auto const visitor_number = yield.get_one(visitor_counter);
			assert(visitor_number);
			accept_handler handler{*visitor_number};
			auto context = Si::apply_visitor(handler, *result);
			if (!context.empty())
			{
				yield(std::move(context));
			}
		}
	}));
	auto all = rx::make_total_consumer(rx::ref(handling_clients));
	all.start();

	auto const thread_count = boost::thread::hardware_concurrency();

	std::vector<boost::unique_future<void>> workers;
	std::generate_n(std::back_inserter(workers), thread_count - 1, [&io]
	{
		return boost::async(boost::launch::async, [&io]
		{
			io.run();
		});
	});
	std::for_each(workers.begin(), workers.end(), [](boost::unique_future<void> &worker)
	{
		worker.get();
	});
	io.run();
}
