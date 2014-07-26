#ifndef SILICIUM_REACTIVE_TCP_ACCEPTOR_HPP
#define SILICIUM_REACTIVE_TCP_ACCEPTOR_HPP

#include <reactive/observable.hpp>
#include <reactive/exchange.hpp>
#include <silicium/fast_variant.hpp>
#include <boost/asio/ip/tcp.hpp>

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
			receiver_ = &receiver;
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

#endif
