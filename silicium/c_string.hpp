#ifndef SILICIUM_C_STRING_HPP
#define SILICIUM_C_STRING_HPP

#include <cassert>
#include <silicium/config.hpp>

namespace Si
{
	struct c_string
	{
		c_string()
			: m_begin(nullptr)
		{
		}

		explicit c_string(char const *begin)
			: m_begin(begin)
		{
			assert(m_begin);
		}

		bool is_set() const BOOST_NOEXCEPT
		{
			return m_begin != nullptr;
		}

		bool empty() const BOOST_NOEXCEPT
		{
			assert(is_set());
			return (*m_begin == '\0');
		}

		char const *c_str() const BOOST_NOEXCEPT
		{
			assert(is_set());
			return m_begin;
		}

	private:

		char const *m_begin;
	};
}

#endif
