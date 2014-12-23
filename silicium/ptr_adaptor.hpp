#ifndef SILICIUM_PTR_ADAPTOR_HPP
#define SILICIUM_PTR_ADAPTOR_HPP

#include <silicium/config.hpp>
#include <boost/core/explicit_operator_bool.hpp>

namespace Si
{
	template <class Pointee>
	struct ptr_adaptor
	{
		template <class ...Args>
		explicit ptr_adaptor(Args &&...args)
			: m_value(std::forward<Args>(args)...)
		{
		}

		Pointee &operator *() BOOST_NOEXCEPT
		{
			return m_value;
		}

		Pointee *operator ->() BOOST_NOEXCEPT
		{
			return &m_value;
		}

		bool operator !() const BOOST_NOEXCEPT
		{
			return false;
		}

		BOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()

#if !SILICIUM_COMPILER_GENERATES_MOVES
		ptr_adaptor(ptr_adaptor &&other)
			: m_value(std::move(other.m_value))
		{
		}

		ptr_adaptor &operator = (ptr_adaptor &&other)
		{
			m_value = std::move(other.m_value);
			return *this;
		}
#endif

	private:

		Pointee m_value;
	};

	template <class Pointee>
	auto make_ptr_adaptor(Pointee &&value)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		->ptr_adaptor<typename std::decay<Pointee>::type>
#endif
	{
		return ptr_adaptor<typename std::decay<Pointee>::type>(std::forward<Pointee>(value));
	}
}

#endif
