#ifndef SILICIUM_RELATIVE_PATH_HPP
#define SILICIUM_RELATIVE_PATH_HPP

#include <silicium/path.hpp>
#include <silicium/noexcept_string.hpp>
#include <boost/filesystem/path.hpp>

namespace Si
{
	struct relative_path
	{
		typedef native_path_char char_type;

		relative_path() BOOST_NOEXCEPT
		{
		}

		explicit relative_path(noexcept_string const &value)
		    : m_value(value)
		{
		}

		explicit relative_path(boost::filesystem::path const &value)
#ifdef _WIN32
		    : m_value(value)
#else
			: m_value(value.c_str())
#endif
		{
		}

		explicit relative_path(char_type const *c_str)
			: m_value(c_str)
		{
		}

		template <std::size_t N>
		explicit relative_path(char_type const (&c_str_literal)[N])
			: m_value(c_str_literal)
		{
		}

#ifdef _WIN32
		explicit relative_path(char const *c_str)
			: m_value(c_str)
		{
		}

		template <std::size_t N>
		explicit relative_path(char const (&c_str_literal)[N])
			: m_value(c_str_literal)
		{
		}
#endif

		template <class Iterator>
		relative_path(Iterator begin, Iterator end)
		    : m_value(begin, end)
		{
		}

		relative_path(relative_path &&other) BOOST_NOEXCEPT
		    : m_value(std::move(other.m_value))
		{
		}

		relative_path(relative_path const &other)
			: m_value(other.m_value)
		{
		}

		relative_path &operator = (relative_path &&other) BOOST_NOEXCEPT
		{
			m_value = std::move(other.m_value);
			return *this;
		}

		relative_path &operator = (relative_path const &other)
		{
			m_value = other.m_value;
			return *this;
		}

		void swap(relative_path &other) BOOST_NOEXCEPT
		{
			m_value.swap(other.m_value);
		}

		boost::filesystem::path
#ifdef _WIN32
		const &
#endif
		to_boost_path() const
		{
			return m_value.to_boost_path();
		}

#ifdef _WIN32
		boost::filesystem::path const &
#else
		noexcept_string const &
#endif
		underlying() const BOOST_NOEXCEPT
		{
			return m_value.underlying();
		}

		char_type const *c_str() const BOOST_NOEXCEPT
		{
			return m_value.c_str();
		}

		bool empty() const BOOST_NOEXCEPT
		{
			return underlying().empty();
		}

	private:

		path m_value;
	};

	inline std::ostream &operator << (std::ostream &out, relative_path const &p)
	{
		return out << p.underlying();
	}

	template <class ComparableToPath>
	inline bool operator == (relative_path const &left, ComparableToPath const &right)
	{
		return left.underlying() == right;
	}

	template <class ComparableToPath>
	inline bool operator == (ComparableToPath const &left, relative_path const &right)
	{
		return left == right.underlying();
	}

	inline bool operator == (relative_path const &left, boost::filesystem::path const &right)
	{
		return right.compare(left.c_str()) == 0;
	}

	inline bool operator == (boost::filesystem::path const &left, relative_path const &right)
	{
		return left.compare(right.c_str()) == 0;
	}

	inline bool operator == (relative_path const &left, relative_path const &right)
	{
		return left.underlying() == right.underlying();
	}

	template <class ComparableToPath>
	inline bool operator != (relative_path const &left, ComparableToPath const &right)
	{
		return !(left == right);
	}

	template <class ComparableToPath>
	inline bool operator != (ComparableToPath const &left, relative_path const &right)
	{
		return !(left == right);
	}

	inline bool operator < (relative_path const &left, relative_path const &right)
	{
		return left.underlying() < right.underlying();
	}

	inline std::size_t hash_value(relative_path const &value)
	{
		using boost::hash_value;
		return hash_value(value.underlying());
	}

	inline relative_path leaf(relative_path const &whole)
	{
		//TODO: do this efficiently
		return relative_path(whole.to_boost_path().leaf());
	}

	inline relative_path parent(relative_path const &whole)
	{
		//TODO: do this efficiently
		return relative_path(whole.to_boost_path().parent_path());
	}

	inline relative_path operator / (relative_path const &front, relative_path const &back)
	{
		//TODO: do this efficiently
		return relative_path(front.to_boost_path() / back.to_boost_path());
	}
}

namespace std
{
	template <>
	struct hash< ::Si::relative_path>
	{
		std::size_t operator()(Si::relative_path const &value) const
		{
			return hash_value(value);
		}
	};
}

#endif
