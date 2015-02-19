#ifndef SILICIUM_C_STRING_HPP
#define SILICIUM_C_STRING_HPP

#include <cassert>
#include <silicium/config.hpp>

namespace Si
{
	template <class Char>
	struct basic_c_string
	{
		typedef Char char_type;

		basic_c_string()
			: m_begin(nullptr)
		{
		}

		explicit basic_c_string(char_type const *begin)
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

		char_type const *c_str() const BOOST_NOEXCEPT
		{
			assert(is_set());
			return m_begin;
		}

	private:

		char_type const *m_begin;
	};

	typedef basic_c_string<char> c_string;
	typedef basic_c_string<wchar_t> cw_string;

	typedef
#ifdef _WIN32
		cw_string
#else
		c_string
#endif
		native_path_string;
}

#endif
