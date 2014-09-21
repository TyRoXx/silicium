#ifndef SILICIUM_REACTIVE_SENDING_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_SENDING_OBSERVABLE_HPP

#include <silicium/observer.hpp>
#include <silicium/exchange.hpp>
#include <silicium/override.hpp>
#include <silicium/error_or.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/range/iterator_range.hpp>

namespace Si
{
	struct sending_observable
	{
		typedef error_or<std::size_t> element_type;
		typedef boost::iterator_range<char const *> buffer_type;

		sending_observable()
			: socket(nullptr)
			, receiver_(nullptr)
		{
		}

		explicit sending_observable(boost::asio::ip::tcp::socket &socket, buffer_type buffer)
			: socket(&socket)
			, buffer(buffer)
			, receiver_(nullptr)
		{
		}

		void async_get_one(observer<element_type> &receiver)
		{
			assert(!receiver_);
			if (buffer.empty())
			{
				return receiver.ended();
			}
			receiver_ = &receiver;
			boost::asio::async_write(*socket, boost::asio::buffer(buffer.begin(), buffer.size()), [this](boost::system::error_code error, std::size_t bytes_sent)
			{
				buffer = buffer_type();
				return error ?
						exchange(receiver_, nullptr)->got_element(error) :
						exchange(receiver_, nullptr)->got_element(bytes_sent);
			});
		}

	private:

		boost::asio::ip::tcp::socket *socket;
		buffer_type buffer;
		observer<element_type> *receiver_;
	};
}

#endif
