#ifndef SILICIUM_ASIO_USE_OBSERVABLE_HPP
#define SILICIUM_ASIO_USE_OBSERVABLE_HPP

#include <boost/version.hpp>
#include <silicium/variant.hpp>

#define SILICIUM_HAS_USE_OBSERVABLE (SILICIUM_HAS_VARIANT && (BOOST_VERSION >= 105400))

#if SILICIUM_HAS_USE_OBSERVABLE
#include <silicium/observable/ptr.hpp>
#include <silicium/observable/erased_observer.hpp>
#include <silicium/error_or.hpp>
#include <algorithm>
#include <boost/asio/async_result.hpp>
#include <boost/system/error_code.hpp>

namespace Si
{
	namespace asio
	{
		struct use_observable_t
		{
			BOOST_CONSTEXPR use_observable_t()
			{
			}
		};

		static BOOST_CONSTEXPR_OR_CONST use_observable_t use_observable;

		namespace detail
		{
			template <class Element>
			struct future_observable
			{
				typedef Element element_type;

				template <class Observer>
				void async_get_one(Observer &&receiver)
				{
					Si::visit<void>(m_waiting_or_storing,
					                [this, &receiver](unit)
					                {
						                m_waiting_or_storing =
						                    erased_observer<Element>(std::forward<Observer>(receiver));
						            },
					                [](erased_observer<Element> &)
					                {
						                SILICIUM_UNREACHABLE();
						            },
					                [&receiver](element_type &stored)
					                {
						                std::forward<Observer>(receiver).got_element(std::move(stored));
						            });
				}

				void fulfill(Element element)
				{
					Si::visit<void>(m_waiting_or_storing,
					                [this, &element](unit)
					                {
						                m_waiting_or_storing = std::move(element);
						            },
					                [this, &element](erased_observer<Element> &receiver)
					                {
						                auto moved_receiver = std::move(receiver);
						                m_waiting_or_storing = unit();
						                std::move(moved_receiver).got_element(std::move(element));
						            },
					                [](element_type &)
					                {
						                SILICIUM_UNREACHABLE();
						            });
				}

			private:
				variant<unit, erased_observer<Element>, Element> m_waiting_or_storing;
			};

			template <class Element>
			struct observable_handler
			{
				explicit observable_handler(use_observable_t)
				    : m_observable(std::make_shared<future_observable<Element>>())
				{
				}

				void operator()(boost::system::error_code ec)
				{
					assert(m_observable);
					m_observable->fulfill(ec);
				}

				template <class A2>
				void operator()(boost::system::error_code ec, A2 &&a2)
				{
					assert(m_observable);
					if (ec)
					{
						m_observable->fulfill(ec);
					}
					else
					{
						m_observable->fulfill(std::forward<A2>(a2));
					}
				}

				std::shared_ptr<future_observable<Element>> const &get_observable() const
				{
					return m_observable;
				}

			private:
				std::shared_ptr<future_observable<Element>> m_observable;
			};
		}
	}
}

namespace boost
{
	namespace asio
	{
		template <class Element>
		struct async_result<Si::asio::detail::observable_handler<Element>>
		{
			typedef Si::ptr_observable<Element, std::shared_ptr<Si::asio::detail::future_observable<Element>>> type;

			explicit async_result(Si::asio::detail::observable_handler<Element> &handler)
			    : m_observable(handler.get_observable())
			{
			}

			type get()
			{
				return type{m_observable};
			}

		private:
			std::shared_ptr<Si::asio::detail::future_observable<Element>> m_observable;
		};

		template <typename ReturnType>
		struct handler_type<Si::asio::use_observable_t, ReturnType(boost::system::error_code)>
		{
			typedef Si::asio::detail::observable_handler<boost::system::error_code> type;
		};

		template <typename ReturnType, class A2>
		struct handler_type<Si::asio::use_observable_t, ReturnType(boost::system::error_code, A2)>
		{
			typedef Si::asio::detail::observable_handler<Si::error_or<A2>> type;
		};
	}
}

#endif

#endif
