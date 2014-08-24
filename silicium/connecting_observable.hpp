#ifndef SILICIUM_CONNECTING_OBSERVABLE_HPP
#define SILICIUM_CONNECTING_OBSERVABLE_HPP

#include <silicium/exchange.hpp>
#include <silicium/observer.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace Si
{
	struct connecting_observable
	{
		typedef boost::system::error_code element_type;

		explicit connecting_observable(boost::asio::ip::tcp::socket &socket, boost::asio::ip::tcp::endpoint destination)
			: socket(&socket)
			, destination(destination)
		{
		}

		void async_get_one(observer<element_type> &receiver)
		{
			assert(socket);
			assert(!receiver_);
			receiver_ = &receiver;
			socket->async_connect(destination, [this](boost::system::error_code ec)
			{
				Si::exchange(receiver_, nullptr)->got_element(ec);
			});
		}

		void cancel()
		{
			assert(receiver_);
			throw std::logic_error("to do");
		}

	private:

		boost::asio::ip::tcp::socket *socket = nullptr;
		boost::asio::ip::tcp::endpoint destination;
		observer<element_type> *receiver_ = nullptr;
	};
}

#endif
