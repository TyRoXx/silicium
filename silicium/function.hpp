#ifndef SILICIUM_FUNCTION_HPP
#define SILICIUM_FUNCTION_HPP

#include <silicium/is_handle.hpp>
#include <silicium/explicit_operator_bool.hpp>
#include <boost/utility/enable_if.hpp>
#include <memory>
#include <cassert>

#define SILICIUM_HAS_FUNCTION SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES
namespace Si
{
#if SILICIUM_HAS_FUNCTION
	template <class Signature>
	struct function;

	template <class Result, class... Args>
	struct function<Result(Args...)>
	{
		function() BOOST_NOEXCEPT
		{
		}

		function(function &&other) BOOST_NOEXCEPT : m_content(std::move(other.m_content))
		{
		}

		function(function const &other) BOOST_NOEXCEPT : m_content(other.m_content)
		{
		}

		function &operator=(function &&other) BOOST_NOEXCEPT
		{
			m_content = std::move(other.m_content);
			return *this;
		}

		function &operator=(function const &other) BOOST_NOEXCEPT
		{
			m_content = other.m_content;
			return *this;
		}

		template <class F>
		function(F &&content, typename boost::enable_if_c<!std::is_same<function, typename std::decay<F>::type>::value,
		                                                  void>::type * = nullptr)
		    : m_content(std::make_shared<holder<typename std::decay<F>::type>>(std::forward<F>(content)))
		{
		}

		bool operator!() const BOOST_NOEXCEPT
		{
			return !m_content;
		}

		SILICIUM_EXPLICIT_OPERATOR_BOOL()

		Result operator()(Args... arguments) const
		{
			assert(m_content);
			return m_content->operator()(std::forward<Args>(arguments)...);
		}

	private:
		struct holder_base
		{
			virtual ~holder_base()
			{
			}
			virtual Result operator()(Args... arguments) = 0;
		};

		template <class F>
		struct holder : holder_base
		{
			BOOST_STATIC_ASSERT((!std::is_same<std::nullptr_t, F>::value));

			template <class G>
			explicit holder(G &&function)
			    : m_function(std::forward<G>(function))
			{
			}

			virtual Result operator()(Args... arguments) SILICIUM_OVERRIDE
			{
				return m_function(std::forward<Args>(arguments)...);
			}

		private:
			F m_function;
		};

		std::shared_ptr<holder_base> m_content;
	};

	BOOST_STATIC_ASSERT(is_handle<function<void()>>::value);
#endif
}

#endif
