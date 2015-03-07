#ifndef SILICIUM_PATH_SEGMENT_HPP
#define SILICIUM_PATH_SEGMENT_HPP

#include <silicium/relative_path.hpp>
#include <silicium/absolute_path.hpp>
#include <silicium/optional.hpp>
#include <boost/filesystem/path.hpp>

namespace Si
{
	struct path_segment
	{
		typedef native_path_char char_type;

		path_segment() BOOST_NOEXCEPT
		{
		}

		path_segment(path_segment &&other) BOOST_NOEXCEPT
		    : m_value(std::move(other.m_value))
		{
		}

		path_segment(path_segment const &other)
			: m_value(other.m_value)
		{
		}

		path_segment &operator = (path_segment &&other) BOOST_NOEXCEPT
		{
			m_value = std::move(other.m_value);
			return *this;
		}

		path_segment &operator = (path_segment const &other)
		{
			m_value = other.m_value;
			return *this;
		}

		void swap(path_segment &other) BOOST_NOEXCEPT
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

		static optional<path_segment> create(boost::filesystem::path const &maybe_segment)
		{
			if (maybe_segment.parent_path().empty())
			{
				return path_segment(maybe_segment);
			}
			return none;
		}

	private:

		path m_value;

		explicit path_segment(boost::filesystem::path const &value)
			: m_value(value)
		{
		}
	};

	inline std::ostream &operator << (std::ostream &out, path_segment const &p)
	{
		return out << p.underlying();
	}

	template <class ComparableToPath>
	inline bool operator == (path_segment const &left, ComparableToPath const &right)
	{
		return left.underlying() == right;
	}

	template <class ComparableToPath>
	inline bool operator == (ComparableToPath const &left, path_segment const &right)
	{
		return left == right.underlying();
	}

	inline bool operator == (path_segment const &left, boost::filesystem::path const &right)
	{
		return right.compare(left.c_str()) == 0;
	}

	inline bool operator == (boost::filesystem::path const &left, path_segment const &right)
	{
		return left.compare(right.c_str()) == 0;
	}

	inline bool operator == (path_segment const &left, path_segment const &right)
	{
		return left.underlying() == right.underlying();
	}

	template <class ComparableToPath>
	inline bool operator != (path_segment const &left, ComparableToPath const &right)
	{
		return !(left == right);
	}

	template <class ComparableToPath>
	inline bool operator != (ComparableToPath const &left, path_segment const &right)
	{
		return !(left == right);
	}

	inline bool operator < (path_segment const &left, path_segment const &right)
	{
		return left.underlying() < right.underlying();
	}

	inline std::size_t hash_value(path_segment const &value)
	{
		using boost::hash_value;
		return hash_value(value.underlying());
	}

	inline relative_path operator / (path_segment const &front, path_segment const &back)
	{
		//TODO: do this efficiently
		relative_path result(front.to_boost_path() / back.to_boost_path());
		return result;
	}

	inline relative_path operator / (relative_path const &front, path_segment const &back)
	{
		return front / relative_path(back.to_boost_path());
	}

	inline absolute_path operator / (absolute_path const &front, path_segment const &back)
	{
		absolute_path result = front;
		result.combine(relative_path(back.to_boost_path()));
		return result;
	}
}

namespace std
{
	template <>
	struct hash< ::Si::path_segment>
	{
		std::size_t operator()(Si::path_segment const &value) const
		{
			return hash_value(value);
		}
	};
}

#endif
