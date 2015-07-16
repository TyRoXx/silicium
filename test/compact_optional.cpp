#include <silicium/optional.hpp>
#include <silicium/noexcept_string.hpp>
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

		compact_optional(value_type value)
			: m_storage(std::move(value))
		{
			assert(*this);
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
	bool operator == (compact_optional<Policy> const &left, typename Policy::value_type const &right)
	{
		if (left)
		{
			return *left == right;
		}
		return false;
	}

	template <class Policy>
	bool operator == (typename Policy::value_type const &left, compact_optional<Policy> const &right)
	{
		if (right)
		{
			return left == *right;
		}
		return false;
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
	bool operator != (compact_optional<Policy> const &left, compact_optional<Policy> const &right)
	{
		return !(left == right);
	}

	template <class Policy>
	bool operator != (compact_optional<Policy> const &left, none_t)
	{
		return !!left;
	}

	template <class Policy>
	bool operator != (none_t, compact_optional<Policy> const &right)
	{
		return !!right;
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

	template <class String>
	struct non_empty_string
	{
		typedef String value_type;
		static bool is_none(value_type const &value)
		{
			return value.empty();
		}
		static value_type get_none()
		{
			return {};
		}
	};

	template <class Pointee>
	struct pointer
	{
		typedef Pointee *value_type;
		static bool is_none(value_type value)
		{
			return (value == get_none());
		}
		static value_type get_none()
		{
			static union
			{
				Pointee a, b;
			}
			none_dummy;
			return &none_dummy.a;
		}
	};

	typedef compact_optional<positive_number<boost::int32_t>> optional_int31;
	BOOST_STATIC_ASSERT(sizeof(optional_int31) == sizeof(boost::uint32_t));

	typedef compact_optional<non_empty_string<noexcept_string>> optional_non_empty_string;
	BOOST_STATIC_ASSERT(sizeof(optional_non_empty_string) == sizeof(noexcept_string));

	template <class Pointee>
	using optional_ptr = compact_optional<pointer<Pointee>>;
	BOOST_STATIC_ASSERT(sizeof(optional_ptr<int>) == sizeof(int *));
}

BOOST_AUTO_TEST_CASE(compact_optional_none_equal)
{
	Si::optional_int31 a, b;
	BOOST_CHECK_EQUAL(a, b);
	BOOST_CHECK_EQUAL(a, Si::none);
	BOOST_CHECK_EQUAL(Si::none, b);
	BOOST_CHECK_EQUAL(Si::none, Si::none);
	BOOST_CHECK_NE(a, std::numeric_limits<boost::int32_t>::max());
	BOOST_CHECK_NE(std::numeric_limits<boost::int32_t>::max(), a);
}

BOOST_AUTO_TEST_CASE(compact_optional_construct)
{
	Si::optional_non_empty_string a;
	Si::optional_non_empty_string b("1");
	BOOST_CHECK_EQUAL(b, b);
	Si::optional_non_empty_string c(Si::noexcept_string("2"));
	BOOST_CHECK_EQUAL(c, c);
	Si::optional_non_empty_string d(Si::some, "3");
	BOOST_CHECK_EQUAL(d, d);
	Si::optional_non_empty_string e(Si::some, Si::noexcept_string("2"));
	Si::optional_non_empty_string f(Si::none);
	BOOST_CHECK_EQUAL(e, e);
	BOOST_CHECK_NE(a, b);
	BOOST_CHECK_NE(a, c);
	BOOST_CHECK_NE(a, d);
	BOOST_CHECK_EQUAL(c, e);
	BOOST_CHECK_EQUAL(a, f);
}

BOOST_AUTO_TEST_CASE(optional_ptr)
{
	Si::optional_ptr<long> a, b;
	BOOST_CHECK_EQUAL(a, b);
	BOOST_CHECK_EQUAL(b, Si::none);
	long pointee = 2;
	b = &pointee;
	BOOST_CHECK_EQUAL(a, Si::none);
	BOOST_CHECK_NE(b, Si::none);
	**b = 3;
	BOOST_CHECK_EQUAL(3, pointee);
}
