#ifndef SILICIUM_REACTIVE_SOCKET_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_SOCKET_OBSERVABLE_HPP

#include <silicium/error_or.hpp>
#include <silicium/override.hpp>
#include <silicium/observable/observer.hpp>
#include <silicium/exchange.hpp>
#include <silicium/iterator_range.hpp>
#include <boost/config.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace Si
{
	typedef iterator_range<char const *> incoming_bytes;
	typedef error_or<incoming_bytes> received_from_socket;

	struct socket_observable
	{
		typedef received_from_socket element_type;
		typedef iterator_range<char *> buffer_type;

		socket_observable()
			: receiver_(nullptr)
		{
		}

		socket_observable(boost::asio::ip::tcp::socket &socket, buffer_type buffer)
			: socket(&socket)
			, buffer(buffer)
			, receiver_(nullptr)
		{
			assert(!buffer.empty());
		}

		void async_get_one(observer<element_type> &receiver)
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

	private:

		boost::asio::ip::tcp::socket *socket;
		buffer_type buffer;
		observer<element_type> *receiver_;
	};
}

#endif
