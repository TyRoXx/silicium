#ifndef SILICIUM_REACTIVE_RECEIVER_HPP
#define SILICIUM_REACTIVE_RECEIVER_HPP

#include <reactive/observable.hpp>

namespace rx
{
	struct receiver : observable<char>
	{
		typedef char element_type;

		explicit receiver(shared_socket socket)
			: impl_(std::make_shared<impl>())
		{
			impl_->socket = std::move(socket);
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!impl_->receiver_);
			impl_->receiver_ = &receiver;
			return start();
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(impl_->receiver_);
			impl_->socket->cancel();
			impl_->receiver_ = nullptr;
		}

	private:

		struct impl
		{
			shared_socket socket;
			observer<element_type> *receiver_ = nullptr;
			char buffer = 0;
		};
		std::shared_ptr<impl> impl_;

		void start()
		{
			std::weak_ptr<impl> weak_impl = impl_;
			impl_->socket->async_receive(
				boost::asio::buffer(&impl_->buffer, 1),
				[weak_impl](boost::system::error_code error, std::size_t bytes_transferred)
			{
				auto locked_impl = weak_impl.lock();
				if (!locked_impl)
				{
					return;
				}
				if (error)
				{
					if (error == boost::asio::error::operation_aborted)
					{
						//was cancelled
						return;
					}
					exchange(locked_impl->receiver_, nullptr)->ended(); //TODO proper error handling?
				}
				else
				{
					assert(bytes_transferred == 1);
					exchange(locked_impl->receiver_, nullptr)->got_element(locked_impl->buffer);
				}
			});
		}
	};
}

#endif
