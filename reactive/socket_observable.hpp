#ifndef SILICIUM_REACTIVE_SOCKET_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_SOCKET_OBSERVABLE_HPP

#include <silicium/fast_variant.hpp>
#include <silicium/override.hpp>
#include <reactive/observable.hpp>
#include <reactive/exchange.hpp>
#include <boost/config.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/range/iterator_range.hpp>

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
}

#endif
