#ifndef SILICIUM_REACTIVE_SENDING_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_SENDING_OBSERVABLE_HPP

#include <reactive/observable.hpp>
#include <reactive/exchange.hpp>
#include <silicium/override.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/range/iterator_range.hpp>

namespace rx
{
	struct sending_observable : observable<boost::system::error_code>
	{
		typedef boost::system::error_code element_type;
		typedef boost::iterator_range<char const *> buffer_type;

		explicit sending_observable(boost::asio::ip::tcp::socket &socket, buffer_type buffer)
			: socket(&socket)
			, buffer(buffer)
		{
		}

		virtual void async_get_one(observer<boost::system::error_code> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			if (buffer.empty())
			{
				return receiver.ended();
			}
			receiver_ = &receiver;
			boost::asio::async_write(*socket, boost::asio::buffer(buffer.begin(), buffer.size()), [this](boost::system::error_code error, std::size_t bytes_sent)
			{
				boost::ignore_unused_variable_warning(bytes_sent);
				assert(buffer.size() == static_cast<ptrdiff_t>(bytes_sent));
				buffer = buffer_type();
				return exchange(receiver_, nullptr)->got_element(error);
			});
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			throw std::logic_error("to do");
		}

	private:

		boost::asio::ip::tcp::socket *socket = nullptr;
		buffer_type buffer;
		observer<boost::system::error_code> *receiver_ = nullptr;
	};
}

#endif
