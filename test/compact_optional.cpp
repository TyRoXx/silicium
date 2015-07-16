#include <silicium/optional.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
	template <class Policy>
	struct compact_optional
	{
		typedef typename Policy::value_type value_type;

		compact_optional()
			: m_storage(Policy::get_none())
		{
			assert(!*this);
		}

		template <class ...Args>
		compact_optional(some_t, Args &&...args)
			: m_storage(std::forward<Args>(args)...)
		{
			assert(*this);
		}

		compact_optional(none_t)
			: m_storage(Policy::get_none())
		{
			assert(!*this);
		}

		bool operator !() const BOOST_NOEXCEPT
		{
			return Policy::is_none(m_storage);
		}

		SILICIUM_EXPLICIT_OPERATOR_BOOL()

		value_type &operator *() BOOST_NOEXCEPT
		{
			assert(*this);
			return m_storage;
		}

		value_type const &operator *() const BOOST_NOEXCEPT
		{
			assert(*this);
			return m_storage;
		}

		value_type *operator ->() BOOST_NOEXCEPT
		{
			assert(*this);
			return &m_storage;
		}

		value_type const *operator ->() const BOOST_NOEXCEPT
		{
			assert(*this);
			return &m_storage;
		}

	private:

		value_type m_storage;
	};

	template <class Policy>
	bool operator == (compact_optional<Policy> const &left, compact_optional<Policy> const &right)
	{
		if (left)
		{
			if (right)
			{
				return *left == *right;
			}
			return false;
		}
		else
		{
			if (right)
			{
				return false;
			}
			return true;
		}
	}

	template <class Policy>
	bool operator == (compact_optional<Policy> const &left, none_t)
	{
		return !left;
	}

	template <class Policy>
	bool operator == (none_t, compact_optional<Policy> const &right)
	{
		return !right;
	}

	template <class Policy>
	std::ostream &operator << (std::ostream &out, compact_optional<Policy> const &value)
	{
		if (value)
		{
			return out << *value;
		}
		return out << none;
	}

	template <class Number>
	struct positive_number
	{
		typedef Number value_type;
		static bool is_none(value_type value)
		{
			return value < 0;
		}
		static value_type get_none()
		{
			return -1;
		}
	};

	typedef compact_optional<positive_number<boost::int32_t>> optional_int31;

	BOOST_STATIC_ASSERT(sizeof(optional_int31) == sizeof(boost::uint32_t));
}

BOOST_AUTO_TEST_CASE(compact_optional_none_equal)
{
	Si::optional_int31 a, b;
	BOOST_CHECK_EQUAL(a, b);
	BOOST_CHECK_EQUAL(a, Si::none);
	BOOST_CHECK_EQUAL(Si::none, b);
	BOOST_CHECK_EQUAL(Si::none, Si::none);
}
