#ifndef SILICIUM_BLOCK_THREAD_HPP
#define SILICIUM_BLOCK_THREAD_HPP

#include <silicium/config.hpp>

#define SILICIUM_HAS_BLOCK_THREAD (BOOST_VERSION >= 105400)

#if SILICIUM_HAS_BLOCK_THREAD
#include <boost/asio/async_result.hpp>
#include <future>

namespace Si
{
	namespace asio
	{
		struct block_thread_t
		{
			BOOST_CONSTEXPR block_thread_t()
			{
			}
		};

		static BOOST_CONSTEXPR_OR_CONST block_thread_t block_thread;

		namespace detail
		{
			template <class Element>
			struct blocking_thread_handler
			{
#if !SILICIUM_COMPILER_GENERATES_MOVES
				blocking_thread_handler(blocking_thread_handler &&other)
				    : m_promised(std::move(other.m_promised))
				{
				}

				blocking_thread_handler &
				operator=(blocking_thread_handler &&other)
				{
					m_promised = std::move(other.m_promised);
					return *this;
				}
#endif

				explicit blocking_thread_handler(block_thread_t)
				{
				}

				void operator()(Element value)
				{
					m_promised.set_value(std::move(value));
				}

				std::future<Element> get_future()
				{
					return m_promised.get_future();
				}

			private:
				std::promise<Element> m_promised;
			};
		}
	}
}

namespace boost
{
	namespace asio
	{
		template <class Element>
		struct async_result<Si::asio::detail::blocking_thread_handler<Element>>
		{
			typedef Element type;

			explicit async_result(
			    Si::asio::detail::blocking_thread_handler<Element> &handler)
			    : m_result(handler.get_future())
			{
			}

			Element get()
			{
				return m_result.get();
			}

		private:
			std::future<Element> m_result;
		};

		template <typename ReturnType, class A2>
		struct handler_type<Si::asio::block_thread_t, ReturnType(A2)>
		{
			typedef Si::asio::detail::blocking_thread_handler<A2> type;
		};
	}
}
#endif

#endif
