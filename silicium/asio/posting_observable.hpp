#ifndef SILICIUM_ASIO_POSTING_OBSERVABLE_HPP
#define SILICIUM_ASIO_POSTING_OBSERVABLE_HPP

#include <silicium/exchange.hpp>
#include <silicium/observable/function_observer.hpp>
#include <boost/asio/io_service.hpp>

namespace Si
{
	namespace asio
	{
		template <class Next>
		struct posting_observable
		{
			typedef typename Next::element_type element_type;

			posting_observable()
			    : m_io(nullptr)
			{
			}

			explicit posting_observable(boost::asio::io_service &io, Next next)
				: m_io(&io)
				, m_next(std::move(next))
			{
			}

			template <class Observer>
			void async_get_one(Observer &&observer_)
			{
				auto keep_io_running = Si::make_unique<boost::asio::io_service::work>(*m_io);
				m_next.async_get_one(
					make_function_observer([this, keep_io_running = std::move(keep_io_running), observer_ = std::forward<Observer>(observer_)](boost::optional<element_type> element) mutable
				{
					m_io->post([element = std::move(element), observer_ = std::forward<Observer>(observer_)]() mutable
					{
						if (element)
						{
							std::forward<Observer>(observer_).got_element(std::move(*element));
						}
						else
						{
							std::forward<Observer>(observer_).ended();
						}
					});
				}));
			}

		private:

			boost::asio::io_service *m_io;
			Next m_next;
		};

		template <class Next>
		auto make_posting_observable(boost::asio::io_service &io, Next &&next)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
			-> posting_observable<typename std::decay<Next>::type>
#endif
		{
			return posting_observable<typename std::decay<Next>::type>(io, std::forward<Next>(next));
		}
	}
}

#endif
