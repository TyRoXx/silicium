#ifndef SILICIUM_ABSOLUTE_PATH_HPP
#define SILICIUM_ABSOLUTE_PATH_HPP

#include <silicium/relative_path.hpp>
#include <boost/filesystem/operations.hpp>

namespace Si
{
	struct absolute_path
	{
		typedef native_path_char char_type;

		absolute_path() BOOST_NOEXCEPT
		{
		}

		explicit absolute_path(noexcept_string const &value)
		    : m_value(value)
		{
		}

		explicit absolute_path(boost::filesystem::path const &value)
#ifdef _WIN32
		    : m_value(value)
#else
			: m_value(value.c_str())
#endif
		{
		}

		explicit absolute_path(char_type const *c_str)
			: m_value(c_str)
		{
		}

		template <class Iterator>
		absolute_path(Iterator begin, Iterator end)
		    : m_value(begin, end)
		{
		}

		absolute_path(absolute_path &&other) BOOST_NOEXCEPT
		    : m_value(std::move(other.m_value))
		{
		}

		absolute_path(absolute_path const &other)
			: m_value(other.m_value)
		{
		}

		absolute_path &operator = (absolute_path &&other) BOOST_NOEXCEPT
		{
			m_value = std::move(other.m_value);
			return *this;
		}

		absolute_path &operator = (absolute_path const &other)
		{
			m_value = other.m_value;
			return *this;
		}

		void swap(absolute_path &other) BOOST_NOEXCEPT
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

		char_type const *c_str() const BOOST_NOEXCEPT
		{
			return m_value.c_str();
		}

	private:

#ifdef _WIN32
		boost::filesystem::path m_value;
#else
		noexcept_string m_value;
#endif
	};

	inline std::ostream &operator << (std::ostream &out, absolute_path const &p)
	{
		return out << p.underlying();
	}

	template <class ComparableToPath>
	inline bool operator == (absolute_path const &left, ComparableToPath const &right)
	{
		return left.underlying() == right;
	}

	template <class ComparableToPath>
	inline bool operator == (ComparableToPath const &left, absolute_path const &right)
	{
		return left == right.underlying();
	}

	inline bool operator == (absolute_path const &left, boost::filesystem::path const &right)
	{
		return right.compare(left.c_str()) == 0;
	}

	inline bool operator == (boost::filesystem::path const &left, absolute_path const &right)
	{
		return left.compare(right.c_str()) == 0;
	}

	inline bool operator == (absolute_path const &left, absolute_path const &right)
	{
		return left.underlying() == right.underlying();
	}

	template <class ComparableToPath>
	inline bool operator != (absolute_path const &left, ComparableToPath const &right)
	{
		return !(left == right);
	}

	template <class ComparableToPath>
	inline bool operator != (ComparableToPath const &left, absolute_path const &right)
	{
		return !(left == right);
	}

	inline bool operator < (absolute_path const &left, absolute_path const &right)
	{
		return left.underlying() < right.underlying();
	}

	inline std::size_t hash_value(absolute_path const &value)
	{
		using boost::hash_value;
		return hash_value(value.underlying());
	}

	inline relative_path leaf(absolute_path const &whole)
	{
		//TODO: do this efficiently
		return relative_path(whole.to_boost_path().leaf());
	}

	inline absolute_path parent(absolute_path const &whole)
	{
		//TODO: do this efficiently
		return absolute_path(whole.to_boost_path().parent_path());
	}

	inline absolute_path operator / (absolute_path const &front, relative_path const &back)
	{
		//TODO: do this efficiently
		return absolute_path(front.to_boost_path() / back.to_boost_path());
	}

	template <std::size_t N>
	inline absolute_path operator / (absolute_path const &front, absolute_path::char_type const (&literal)[N])
	{
		return front / relative_path(boost::filesystem::path(&literal[0]));
	}

	inline absolute_path get_current_working_directory()
	{
		return absolute_path(boost::filesystem::current_path());
	}

	inline boost::system::error_code remove_file(absolute_path const &name)
	{
		boost::system::error_code ec;
		boost::filesystem::remove(name.to_boost_path(), ec);
		return ec;
	}
}

namespace std
{
	template <>
	struct hash< ::Si::absolute_path>
	{
		std::size_t operator()(Si::absolute_path const &value) const
		{
			return hash_value(value);
		}
	};
}

#endif
