#ifndef SILICIUM_FUNCTION_HPP
#define SILICIUM_FUNCTION_HPP

#include <silicium/config.hpp>
#include <boost/utility/enable_if.hpp>
#include <memory>
#include <cassert>
#if BOOST_VERSION >= 105500
#	include <boost/utility/explicit_operator_bool.hpp>
#endif

namespace Si
{
	template <class Signature>
	struct function;

	template <class Result, class ...Args>
	struct function<Result (Args...)>
	{
		function() BOOST_NOEXCEPT
		{
		}

		function(function &&other) BOOST_NOEXCEPT
		    : m_content(std::move(other.m_content))
		{
		}

		function(function const &other) BOOST_NOEXCEPT
		    : m_content(other.m_content)
		{
		}

		function &operator = (function &&other) BOOST_NOEXCEPT
		{
			m_content = std::move(other.m_content);
			return *this;
		}

		function &operator = (function const &other) BOOST_NOEXCEPT
		{
			m_content = other.m_content;
			return *this;
		}

		template <class F>
		function(F &&content, typename boost::enable_if_c<!std::is_same<function, typename std::decay<F>::type>::value, void>::type * = nullptr)
		    : m_content(std::make_shared<holder<typename std::decay<F>::type>>(std::forward<F>(content)))
		{
		}

		bool operator !() const BOOST_NOEXCEPT
		{
			return !m_content;
		}

#ifdef BOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT
		//the noexcept version was added in 1.56 http://www.boost.org/doc/libs/1_57_0/libs/core/doc/html/core/explicit_operator_bool.html
		BOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()
#elif defined(BOOST_EXPLICIT_OPERATOR_BOOL)
		BOOST_EXPLICIT_OPERATOR_BOOL()
#else
		operator bool() const BOOST_NOEXCEPT
		{
			return m_content != nullptr;
		}
#endif

		Result operator()(Args ...arguments) const
		{
			assert(m_content);
			return m_content->operator ()(std::forward<Args>(arguments)...);
		}

	private:

		struct holder_base
		{
			virtual ~holder_base()
			{
			}
			virtual Result operator()(Args ...arguments) = 0;
		};

		template <class F>
		struct holder : holder_base
		{
			template <class G>
			explicit holder(G &&function)
			    : m_function(std::forward<G>(function))
			{
			}

			virtual Result operator()(Args ...arguments) SILICIUM_OVERRIDE
			{
				return m_function(std::forward<Args>(arguments)...);
			}

		private:

			F m_function;
		};

		std::shared_ptr<holder_base> m_content;
	};
}

#endif