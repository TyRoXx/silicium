#ifndef SILICIUM_REACTIVE_CONNECTOR_HPP
#define SILICIUM_REACTIVE_CONNECTOR_HPP

#include <silicium/observable.hpp>
#include <boost/asio.hpp>
#include <boost/variant.hpp>
#include <silicium/override.hpp>

namespace Si
{
	typedef std::shared_ptr<boost::asio::ip::tcp::socket> shared_socket;
	typedef boost::variant<shared_socket, boost::system::error_code> connector_element;

	template <class SharedSocketFactory, class EndpointObservable>
	struct connector
			: public observable<connector_element>
			, private observer<boost::asio::ip::tcp::endpoint>
	{
		typedef connector_element element_type;

		explicit connector(SharedSocketFactory create_socket, EndpointObservable destination)
			: impl_(std::make_shared<impl>())
		{
			impl_->create_socket = std::move(create_socket);
			impl_->destination = std::move(destination);
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!impl_->receiver_);
			impl_->receiver_ = &receiver;
			return impl_->destination.async_get_one(*this);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(impl_->receiver_);
			if (impl_->connecting)
			{
				impl_->connecting->cancel();
			}
			else
			{
				impl_->destination.cancel();
			}
		}

	private:

		struct impl
		{
			SharedSocketFactory create_socket;
			EndpointObservable destination;
			observer<element_type> *receiver_ = nullptr;
			shared_socket connecting;
		};

		std::shared_ptr<impl> impl_;

		virtual void got_element(boost::asio::ip::tcp::endpoint value) SILICIUM_OVERRIDE
		{
			impl_->fetching_destination = false;
			auto socket = impl_->create_socket();
			std::weak_ptr<impl> weak_impl = impl_;
			socket->async_connect(value, [weak_impl, socket](boost::system::error_code error)
			{
				auto const impl_locked = weak_impl.lock();
				if (!impl_locked)
				{
					return;
				}
				connector_element result;
				if (error)
				{
					if (error == boost::asio::error::operation_aborted)
					{
						return;
					}
					result = error;
				}
				else
				{
					result = socket;
				}
				exchange(impl_locked->receiver_, nullptr)->got_element(std::move(result));
			});
			impl_->connecting = socket;
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			impl_->fetching_destination = false;
			exchange(impl_->receiver_, nullptr)->ended();
		}
	};

	template <class SharedSocketFactory, class EndpointObservable>
	auto connect(SharedSocketFactory create_socket, EndpointObservable destination) -> connector<
		typename std::decay<SharedSocketFactory>::type,
		typename std::decay<EndpointObservable>::type>
	{
		return connector<
				typename std::decay<SharedSocketFactory>::type,
				typename std::decay<EndpointObservable>::type>
				(std::forward<SharedSocketFactory>(create_socket), std::forward<EndpointObservable>(destination));
	}

}

#endif
