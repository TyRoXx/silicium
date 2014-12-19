#ifndef SILICIUM_ASIO_TCP_ACCEPTOR_HPP
#define SILICIUM_ASIO_TCP_ACCEPTOR_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/exchange.hpp>
#include <silicium/error_or.hpp>
#include <silicium/ptr_adaptor.hpp>
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
			{
			}

			explicit tcp_acceptor(AcceptorPtrLike underlying)
				: underlying(std::move(underlying))
			{
			}

			tcp_acceptor(tcp_acceptor &&other) BOOST_NOEXCEPT
				: underlying(std::move(other.underlying))
				, next_client(std::move(other.next_client))
			{
			}

			tcp_acceptor &operator = (tcp_acceptor &&other) BOOST_NOEXCEPT
			{
				underlying = std::move(other.underlying);
				next_client = std::move(other.next_client);
				return *this;
			}

			~tcp_acceptor()
			{
			}

			template <class Observer>
			void async_get_one(Observer &&receiver)
			{
				assert(underlying);
				next_client = std::make_shared<boost::asio::ip::tcp::socket>(underlying->get_io_service());
				underlying->async_accept(*next_client, [this, receiver](boost::system::error_code error) mutable
				{
					if (error)
					{
						if (error == boost::asio::error::operation_aborted)
						{
							return;
						}
						std::forward<Observer>(receiver).got_element(tcp_acceptor_result{error});
					}
					else
					{
						std::forward<Observer>(receiver).got_element(tcp_acceptor_result{std::move(next_client)});
					}
				});
			}

		private:

			AcceptorPtrLike underlying;
			std::shared_ptr<boost::asio::ip::tcp::socket> next_client;

			SILICIUM_DELETED_FUNCTION(tcp_acceptor(tcp_acceptor const &))
			SILICIUM_DELETED_FUNCTION(tcp_acceptor &operator = (tcp_acceptor const &))
		};

		template <class AcceptorPtrLike>
		auto make_tcp_acceptor(AcceptorPtrLike &&acceptor)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			-> tcp_acceptor<typename std::decay<AcceptorPtrLike>::type>
#endif
		{
			return tcp_acceptor<typename std::decay<AcceptorPtrLike>::type>(std::forward<AcceptorPtrLike>(acceptor));
		}

		template <class Protocol, class Service>
		auto make_tcp_acceptor(boost::asio::basic_socket_acceptor<Protocol, Service> &&acceptor)
		{
			return make_tcp_acceptor(make_ptr_adaptor(std::move(acceptor)));
		}
	}
}

#endif
