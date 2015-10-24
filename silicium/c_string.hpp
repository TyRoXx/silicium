#ifndef SILICIUM_C_STRING_HPP
#define SILICIUM_C_STRING_HPP

#include <cassert>
#include <silicium/is_handle.hpp>

namespace Si
{
	template <class Char>
	struct basic_c_string
	{
		typedef Char char_type;

		basic_c_string() BOOST_NOEXCEPT : m_begin(nullptr)
		{
		}

		explicit basic_c_string(char_type const *begin)
		    : m_begin(begin)
		{
			assert(m_begin);
		}

		template <size_t N>
		basic_c_string(char_type const(&literal)[N])
		    : m_begin(&literal[0])
		{
		}

		SILICIUM_USE_RESULT
		bool is_set() const BOOST_NOEXCEPT
		{
			return m_begin != nullptr;
		}

		SILICIUM_USE_RESULT
		bool empty() const BOOST_NOEXCEPT
		{
			assert(is_set());
			return (*m_begin == '\0');
		}

		SILICIUM_USE_RESULT
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

	BOOST_STATIC_ASSERT(is_handle<c_string>::value);
	BOOST_STATIC_ASSERT(is_handle<cw_string>::value);

	typedef
#ifdef _WIN32
	    cw_string
#else
	    c_string
#endif
	        os_c_string;

	typedef os_c_string native_path_string;

#ifdef _WIN32
#define SILICIUM_OS_STR(x) L##x
#else
#define SILICIUM_OS_STR(x) x
#endif

/// this macro is deprecated, use SILICIUM_OS_STR directly instead
#define SILICIUM_SYSTEM_LITERAL SILICIUM_OS_STR
}

#endif
