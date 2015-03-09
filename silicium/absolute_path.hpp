#ifndef SILICIUM_ABSOLUTE_PATH_HPP
#define SILICIUM_ABSOLUTE_PATH_HPP

#include <silicium/relative_path.hpp>
#include <silicium/optional.hpp>
#include <boost/filesystem/operations.hpp>

namespace Si
{
	struct absolute_path
	{
		typedef native_path_char char_type;

		absolute_path() BOOST_NOEXCEPT
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

		void combine(relative_path const &back)
		{
			//TODO: optimize
			*this = absolute_path(to_boost_path() / back.to_boost_path());
		}

		static optional<absolute_path> create(boost::filesystem::path const &maybe_absolute)
		{
			if (maybe_absolute.is_absolute())
			{
				return absolute_path(maybe_absolute);
			}
			return none;
		}

		static optional<absolute_path> create(noexcept_string const &maybe_absolute)
		{
			return create(boost::filesystem::path(maybe_absolute.c_str()));
		}

	private:

		path m_value;

		explicit absolute_path(boost::filesystem::path const &value)
			: m_value(value)
		{
		}
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

	inline optional<absolute_path> parent(absolute_path const &whole)
	{
		//TODO: do this efficiently
		auto boosted = whole.to_boost_path();
		if (boosted.has_parent_path())
		{
			return *absolute_path::create(boosted.parent_path());
		}
		return none;
	}

	inline absolute_path operator / (absolute_path const &front, relative_path const &back)
	{
		//TODO: do this efficiently
		absolute_path result = front;
		result.combine(back);
		return result;
	}

	template <std::size_t N>
	inline absolute_path operator / (absolute_path const &front, absolute_path::char_type const (&literal)[N])
	{
		return front / relative_path(boost::filesystem::path(&literal[0]));
	}

#ifdef _WIN32
	template <std::size_t N>
	inline absolute_path operator / (absolute_path const &front, char const (&literal)[N])
	{
		return front / relative_path(boost::filesystem::path(&literal[0]));
	}
#endif

	inline absolute_path get_current_working_directory()
	{
		return *absolute_path::create(boost::filesystem::current_path());
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
