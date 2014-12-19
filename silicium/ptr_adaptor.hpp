#ifndef SILICIUM_PTR_ADAPTOR_HPP
#define SILICIUM_PTR_ADAPTOR_HPP

#include <silicium/config.hpp>
#include <boost/core/explicit_operator_bool.hpp>

namespace Si
{
	template <class Pointee>
	struct ptr_adaptor
	{
		ptr_adaptor()
		{
		}

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

	private:

		Pointee m_value;
	};

	template <class Pointee>
	auto make_ptr_adaptor(Pointee &&value)
	{
		return ptr_adaptor<typename std::decay<Pointee>::type>(std::forward<Pointee>(value));
	}
}

#endif
