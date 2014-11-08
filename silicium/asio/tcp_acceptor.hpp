#ifndef SILICIUM_ASIO_TCP_ACCEPTOR_HPP
#define SILICIUM_ASIO_TCP_ACCEPTOR_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/exchange.hpp>
#include <silicium/override.hpp>
#include <silicium/fast_variant.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace Si
{
	typedef Si::fast_variant<
		std::shared_ptr<boost::asio::ip::tcp::socket>, //until socket itself is noexcept-movable
		boost::system::error_code
	> tcp_acceptor_result;

	struct tcp_acceptor : private boost::noncopyable
	{
		typedef tcp_acceptor_result element_type;

		tcp_acceptor()
			: underlying(nullptr)
			, receiver_(nullptr)
		{
		}

		explicit tcp_acceptor(boost::asio::ip::tcp::acceptor &underlying)
			: underlying(&underlying)
			, receiver_(nullptr)
		{
		}

		~tcp_acceptor()
		{
			if (!receiver_ || !underlying)
			{
				return;
			}
			underlying->cancel();
		}

		void async_get_one(observer<element_type> &receiver)
		{
			assert(!receiver_);
			assert(underlying);
			next_client = std::make_shared<boost::asio::ip::tcp::socket>(underlying->get_io_service());
			receiver_ = &receiver;
			underlying->async_accept(*next_client, [this](boost::system::error_code error)
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
					Si::exchange(this->receiver_, nullptr)->got_element(tcp_acceptor_result{error});
				}
				else
				{
					Si::exchange(this->receiver_, nullptr)->got_element(tcp_acceptor_result{std::move(next_client)});
				}
			});
		}

	private:

		boost::asio::ip::tcp::acceptor *underlying;
		std::shared_ptr<boost::asio::ip::tcp::socket> next_client;
		observer<element_type> *receiver_;
	};
}

#endif
