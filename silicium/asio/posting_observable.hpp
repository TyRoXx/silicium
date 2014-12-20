#ifndef SILICIUM_ASIO_POSTING_OBSERVABLE_HPP
#define SILICIUM_ASIO_POSTING_OBSERVABLE_HPP

#include <silicium/exchange.hpp>
#include <silicium/observable/observer.hpp>
#include <boost/asio/io_service.hpp>

namespace Si
{
	namespace asio
	{

		template <class Next>
		struct posting_observable : private observer<typename Next::element_type>
		{
			typedef typename Next::element_type element_type;

			explicit posting_observable(boost::asio::io_service &io, Next next)
				: m_io(&io)
				, m_observer(nullptr)
				, m_next(std::move(next))
			{
			}

			template <class Observer>
			void async_get_one(Observer &&observer_)
			{
				m_observer = observer_.get();
				m_next.async_get_one(extend(std::forward<Observer>(observer_), observe_by_ref(static_cast<observer<element_type> &>(*this))));
			}

		private:

			boost::asio::io_service *m_io;
			observer<element_type> *m_observer;
			Next m_next;

			virtual void got_element(element_type value) SILICIUM_OVERRIDE
			{
				auto observer_ = Si::exchange(m_observer, nullptr);
				m_io->post([observer_, value = std::move(value)]() mutable
				{
					observer_->got_element(std::move(value));
				});
			}

			virtual void ended() SILICIUM_OVERRIDE
			{
				auto observer_ = Si::exchange(m_observer, nullptr);
				m_io->post([observer_]() mutable
				{
					observer_->ended();
				});
			}
		};

		template <class Next>
		auto make_posting_observable(boost::asio::io_service &io, Next &&next)
		{
			return posting_observable<typename std::decay<Next>::type>(io, std::forward<Next>(next));
		}
	}
}

#endif
