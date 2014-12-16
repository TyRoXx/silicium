#ifndef SILICIUM_ASIO_TCP_ACCEPTOR_HPP
#define SILICIUM_ASIO_TCP_ACCEPTOR_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/exchange.hpp>
#include <silicium/error_or.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace Si
{
	namespace asio
	{
		typedef error_or<std::shared_ptr<boost::asio::ip::tcp::socket>> tcp_acceptor_result; //until socket itself is noexcept-movable

		template <class AcceptorPtrLike>
		struct tcp_acceptor
		{
			typedef tcp_acceptor_result element_type;

			tcp_acceptor()
				: underlying(nullptr)
				, receiver_(nullptr)
			{
			}

			explicit tcp_acceptor(AcceptorPtrLike underlying)
				: underlying(std::move(underlying))
				, receiver_(nullptr)
			{
			}

			tcp_acceptor(tcp_acceptor &&other) BOOST_NOEXCEPT
				: underlying(std::move(other.underlying))
				, next_client(std::move(other.next_client))
				, receiver_(other.receiver_)
			{
			}

			tcp_acceptor &operator = (tcp_acceptor &&other) BOOST_NOEXCEPT
			{
				underlying = std::move(other.underlying);
				next_client = std::move(other.next_client);
				receiver_ = other.receiver_;
				return *this;
			}

			~tcp_acceptor()
			{
				if (!receiver_ || !underlying)
				{
					return;
				}
				underlying->cancel();
			}

			void async_get_one(ptr_observer<observer<element_type>> receiver)
			{
				assert(!receiver_);
				assert(underlying);
				next_client = std::make_shared<boost::asio::ip::tcp::socket>(underlying->get_io_service());
				receiver_ = receiver.get();
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

			AcceptorPtrLike underlying;
			std::shared_ptr<boost::asio::ip::tcp::socket> next_client;
			observer<element_type> *receiver_;

			SILICIUM_DELETED_FUNCTION(tcp_acceptor(tcp_acceptor const &))
			SILICIUM_DELETED_FUNCTION(tcp_acceptor &operator = (tcp_acceptor const &))
		};

		template <class AcceptorPtrLike>
		auto make_tcp_acceptor(AcceptorPtrLike &&acceptor)
		{
			return tcp_acceptor<typename std::decay<AcceptorPtrLike>::type>(std::forward<AcceptorPtrLike>(acceptor));
		}
	}
}

#endif
