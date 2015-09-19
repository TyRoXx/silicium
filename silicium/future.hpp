#ifndef SILICIUM_FUTURE_HPP
#define SILICIUM_FUTURE_HPP

#include <silicium/variant.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/throw_exception.hpp>

#define SILICIUM_HAS_FUTURE SILICIUM_HAS_VARIANT

#if SILICIUM_HAS_FUTURE
namespace Si
{
	namespace detail
	{
		struct link
		{
			link() BOOST_NOEXCEPT
				: m_other_side(nullptr)
			{
			}

			explicit link(link &other_side)
				: m_other_side(&other_side)
			{
				assert(!other_side.m_other_side);
				other_side.m_other_side = this;
			}

			~link() BOOST_NOEXCEPT
			{
				if (!m_other_side)
				{
					return;
				}
				m_other_side->m_other_side = nullptr;
			}

			SILICIUM_DISABLE_COPY(link)
public:

			link(link &&other) BOOST_NOEXCEPT
				: m_other_side(other.m_other_side)
			{
				other.m_other_side = nullptr;
				m_other_side->m_other_side = this;
			}

			link &operator = (link &&other) BOOST_NOEXCEPT
			{
				using boost::swap;
				swap(m_other_side->m_other_side, other.m_other_side->m_other_side);
				swap(m_other_side, other.m_other_side);
				return *this;
			}

			link *get_other_side() const BOOST_NOEXCEPT
			{
				return m_other_side;
			}

		private:

			link *m_other_side;
		};
	}

	template <class T>
	struct future
	{
		future()
		{
		}

		explicit future(T value)
			: m_state(std::move(value))
		{
		}

		explicit future(detail::link promise)
			: m_state(std::move(promise))
		{
		}

		template <class Handler>
		auto async_wait(Handler &&handler)
			-> typename boost::asio::async_result<typename boost::asio::handler_type<Handler, void(T)>::type>::type
		{
			typename boost::asio::handler_type<Handler, void(T)>::type real_handler(std::forward<Handler>(handler));
			typename boost::asio::async_result<decltype(real_handler)> result(real_handler);
			visit<void>(
				m_state,
				[](empty)
				{
					boost::throw_exception(std::logic_error("to do"));
				},
				[this, &real_handler](T &value)
				{
					real_handler(std::forward<T>(value));
					m_state = empty();
				},
				[](detail::link &)
				{
					boost::throw_exception(std::logic_error("to do"));
				}
			);
			return result.get();
		}

		bool valid() const BOOST_NOEXCEPT
		{
			return try_get_ptr<empty>(m_state) == nullptr;
		}

	private:

		struct empty
		{
		};

		variant<empty, T, detail::link> m_state;
	};

	template <class T>
	future<typename std::decay<T>::type> make_ready_future(T &&value)
	{
		return future<typename std::decay<T>::type>(std::forward<T>(value));
	}

	template <class T>
	struct promise
	{
		future<T> get_future()
		{
			return visit<future<T>>(
				m_state,
				[this](empty) -> future<T>
				{
					detail::link first;
					detail::link second(first);
					m_state = std::move(first);
					return future<T>(std::move(second));
				},
				[this](T &value)
				{
					T result = std::move(value);
					m_state = empty();
					return future<T>(std::move(result));
				},
				[](detail::link &) -> future<T>
				{
					boost::throw_exception(std::logic_error("get_future can be called only once"));
				}
			);
		}

		template <class ...Args>
		void set_value(Args &&...args)
		{
			m_state = T(std::forward<Args>(args)...);
		}

	private:

		struct empty
		{
		};

		variant<empty, T, detail::link> m_state;
	};

	namespace asio
	{
		struct block_thread_t
		{
			BOOST_CONSTEXPR block_thread_t()
			{}
		};

		static BOOST_CONSTEXPR_OR_CONST block_thread_t block_thread;

		namespace detail
		{
			template <class Element>
			struct blocking_thread_handler
			{
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

			explicit async_result(Si::asio::detail::blocking_thread_handler<Element> &handler)
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
