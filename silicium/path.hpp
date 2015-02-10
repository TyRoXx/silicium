#ifndef SILICIUM_PATH_HPP
#define SILICIUM_PATH_HPP

#include <silicium/noexcept_string.hpp>
#include <boost/filesystem/path.hpp>

namespace Si
{
	struct path
	{
		path() BOOST_NOEXCEPT
		{
		}

		explicit path(noexcept_string const &value)
		    : m_value(value)
		{
		}

		explicit path(boost::filesystem::path const &value)
#ifdef _WIN32
		    : m_value(value)
#else
			: m_value(value.c_str())
#endif
		{
		}

		template <class Iterator>
		path(Iterator begin, Iterator end)
		    : m_value(begin, end)
		{
		}

		path(path &&other) BOOST_NOEXCEPT
		    : m_value(std::move(other.m_value))
		{
		}

		path &operator = (path &&other) BOOST_NOEXCEPT
		{
			m_value = std::move(other.m_value);
			return *this;
		}

		void swap(path &other) BOOST_NOEXCEPT
		{
			m_value.swap(other.m_value);
		}

		boost::filesystem::path
#ifdef _WIN32
		const &
#endif
		to_boost_path() const
		{
			return m_value
#ifndef _WIN32
			        .c_str()
#endif
			;
		}

#ifdef _WIN32
		boost::filesystem::path const &
#else
		noexcept_string const &
#endif
		underlying() const BOOST_NOEXCEPT
		{
			return m_value;
		}

	private:

#ifdef _WIN32
		boost::filesystem::path m_value;
#else
		noexcept_string m_value;
#endif
	};

	inline std::ostream &operator << (std::ostream &out, path const &p)
	{
		return out << p.underlying();
	}
}

#endif
