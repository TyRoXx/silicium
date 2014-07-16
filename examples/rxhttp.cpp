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
}

namespace
{
	struct accept_handler : boost::static_visitor<bool>
	{
		bool operator()(std::shared_ptr<boost::asio::ip::tcp::socket> client) const
		{
			return true;
		}

		bool operator()(boost::system::error_code error) const
		{
			return false;
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
			bool continue_ = Si::apply_visitor(handler, *result);
			if (!continue_)
			{
				break;
			}
		}
	});
	auto handle_all = rx::make_total_consumer(rx::ref(handling_clients));
	handle_all.start();
	io.run();
}
