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
				auto keep_io_running =
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
					Si::make_unique
#else
					std::make_shared
#endif
					<boost::asio::io_service::work>(*m_io);
				m_next.async_get_one(
					make_function_observer([this, keep_io_running
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
					= std::move(keep_io_running)
#endif
					, observer_
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
					= std::forward<Observer>(observer_)
#endif
					](Si::optional<element_type> element) mutable
				{
					if (element)
					{
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
						m_io->post([
							element = std::move(element),
							observer_ = std::forward<Observer>(observer_)
						]() mutable
						{
							std::forward<Observer>(observer_).got_element(std::move(*element));
						});
#else
						auto copyable_element = to_shared(std::move(*element));
						m_io->post([
							copyable_element,
							observer_
						]() mutable
						{
							std::forward<Observer>(observer_).got_element(std::move(*copyable_element));
						});
#endif
					}
					else
					{
						m_io->post([observer_
#if SILICIUM_COMPILER_HAS_EXTENDED_CAPTURE
							= std::forward<Observer>(observer_)
#endif
						]() mutable
						{
							std::forward<Observer>(observer_).ended();
						});
					}
				}));
			}

#if SILICIUM_COMPILER_GENERATES_MOVES
			posting_observable(posting_observable &&) = default;
			posting_observable &operator = (posting_observable &&) = default;
#else
			posting_observable(posting_observable &&other)
				: m_io(std::move(other.m_io))
				, m_next(std::move(other.m_next))
			{
			}

			posting_observable &operator = (posting_observable &&other)
			{
				m_io = std::move(other.m_io);
				m_next = std::move(other.m_next);
				return *this;
			}
#endif

		private:

			boost::asio::io_service *m_io;
			Next m_next;

			SILICIUM_DELETED_FUNCTION(posting_observable(posting_observable const &))
			SILICIUM_DELETED_FUNCTION(posting_observable &operator = (posting_observable const &))
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
