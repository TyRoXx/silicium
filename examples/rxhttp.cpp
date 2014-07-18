#include <reactive/coroutine.hpp>
#include <silicium/http/http.hpp>
#include <silicium/fast_variant.hpp>
#include <reactive/total_consumer.hpp>
#include <boost/asio.hpp>
#include <boost/ref.hpp>

namespace rx
{
	using tcp_acceptor_result = Si::fast_variant<
		std::shared_ptr<boost::asio::ip::tcp::socket>, //until socket itself is noexcept-movable
		boost::system::error_code
	>;

	struct tcp_acceptor : observable<tcp_acceptor_result>, boost::noncopyable
	{
		typedef tcp_acceptor_result element_type;

		explicit tcp_acceptor(boost::asio::ip::tcp::acceptor &underlying)
			: underlying(underlying)
		{
		}

		~tcp_acceptor()
		{
			if (!receiver_)
			{
				return;
			}
			underlying.cancel();
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			next_client = std::make_shared<boost::asio::ip::tcp::socket>(underlying.get_io_service());
			underlying.async_accept(*next_client, [this](boost::system::error_code error)
			{
				if (!this->receiver_)
				{
					//can happen when cancel has been called on the observable when the callback was
					//already posted to the io_service
					return;
				}
				if (error)
				{
					if (error == boost::asio::error::operation_aborted)
					{
						return;
					}
					exchange(this->receiver_, nullptr)->got_element(tcp_acceptor_result{error});
				}
				else
				{
					exchange(this->receiver_, nullptr)->got_element(tcp_acceptor_result{std::move(next_client)});
				}
			});
			receiver_ = &receiver;
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			underlying.cancel();
			receiver_ = nullptr;
		}

	private:

		boost::asio::ip::tcp::acceptor &underlying;
		std::shared_ptr<boost::asio::ip::tcp::socket> next_client;
		observer<element_type> *receiver_ = nullptr;
	};

	template <class Observable>
	struct observable_source : Si::source<typename Observable::element_type>
	{
		typedef typename Observable::element_type element_type;

		observable_source(Observable input, yield_context<element_type> &yield)
			: input(std::move(input))
			, yield(yield)
		{
		}

		virtual boost::iterator_range<element_type const *> map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			(void)size;
			return {};
		}

		virtual element_type *copy_next(boost::iterator_range<element_type *> destination) SILICIUM_OVERRIDE
		{
			auto i = begin(destination);
			for (; i != end(destination); ++i)
			{
				auto element = yield.get_one(input);
				if (!element)
				{
					break;
				}
				*i = std::move(*element);
			}
			return i;
		}

		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE
		{
			return 0;
		}

		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE
		{
			return boost::none;
		}

		virtual std::size_t skip(std::size_t count) SILICIUM_OVERRIDE
		{
			size_t skipped = 0;
			while ((skipped < count) && yield.get_one(input))
			{
			}
			return skipped;
		}

	private:

		Observable input;
		yield_context<element_type> &yield;
	};

	template <class Observable, class YieldContext>
	auto make_observable_source(Observable &&input, YieldContext &yield)
	{
		return observable_source<typename std::decay<Observable>::type>(std::forward<Observable>(input), yield);
	}

	typedef Si::fast_variant<std::size_t, boost::system::error_code> received_from_socket;

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
			socket->async_receive(boost::asio::buffer(buffer.begin(), buffer.size()), [this](boost::system::error_code error, std::size_t bytes_received)
			{
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
					exchange(this->receiver_, nullptr)->got_element(bytes_received);
				}
			});
			receiver_ = &receiver;
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
}

namespace
{
	struct accept_handler : boost::static_visitor<rx::detail::nothing>
	{
		rx::detail::nothing operator()(std::shared_ptr<boost::asio::ip::tcp::socket> client) const
		{
			throw std::logic_error("not implemented");
		}

		rx::detail::nothing operator()(boost::system::error_code error) const
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
	auto handling_clients = rx::make_coroutine<rx::detail::nothing>([&clients](rx::yield_context<rx::detail::nothing> &yield) -> void
	{
		for (;;)
		{
			auto result = yield.get_one(clients);
			if (!result)
			{
				break;
			}
			accept_handler handler;
			yield(Si::apply_visitor(handler, *result));
		}
	});
	auto all = rx::make_total_consumer(rx::ref(handling_clients));
	all.start();
	io.run();
}
