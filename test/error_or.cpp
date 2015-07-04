#include <silicium/error_or.hpp>
#include <silicium/noexcept_string.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/unordered_map.hpp>
#include <boost/container/map.hpp>
#include <system_error>
#include <unordered_map>
#include <map>

BOOST_AUTO_TEST_CASE(error_or_from_value)
{
	Si::error_or<int> value(2);
	BOOST_CHECK(!value.is_error());
	BOOST_REQUIRE(value.get_ptr());
	BOOST_CHECK_EQUAL(boost::system::error_code(), value.error());
	BOOST_CHECK_EQUAL(2, *value.get_ptr());
	BOOST_CHECK_EQUAL(2, value.get());
}

BOOST_AUTO_TEST_CASE(error_or_from_value_const)
{
	Si::error_or<int> const value(2);
	BOOST_CHECK(!value.is_error());
	BOOST_CHECK_EQUAL(2, value.get());
	BOOST_REQUIRE(value.get_ptr());
	BOOST_CHECK_EQUAL(2, *value.get_ptr());
}

BOOST_AUTO_TEST_CASE(error_or_convert_from_value)
{
	Si::error_or<Si::noexcept_string> const value("C string literal");
	BOOST_CHECK(!value.is_error());
	BOOST_CHECK_EQUAL("C string literal", value.get());
}

BOOST_AUTO_TEST_CASE(error_or_movable_only)
{
	Si::error_or<std::unique_ptr<int>> value(Si::make_unique<int>(2));
	BOOST_REQUIRE(value.get_ptr());
	BOOST_REQUIRE(*value.get_ptr());
	BOOST_CHECK_EQUAL(2, **value.get_ptr());
	std::unique_ptr<int> v = std::move(value.get());
	BOOST_CHECK(!value.get());
	BOOST_CHECK(!value.is_error());
	BOOST_CHECK_EQUAL(boost::system::error_code(), value.error());
	BOOST_CHECK_EQUAL(2, *v);
}

struct expected_exception : std::exception
{
};

struct throws_on_copy_construction
{
	bool active;

	throws_on_copy_construction()
		: active(true)
	{
	}

	throws_on_copy_construction(throws_on_copy_construction const &other)
		: active(other.active)
	{
		if (active)
		{
			throw expected_exception();
		}
	}

	throws_on_copy_construction &operator = (throws_on_copy_construction const &) = delete;
};

#if SILICIUM_HAS_EXCEPTIONS
BOOST_AUTO_TEST_CASE(error_or_copy_construction_from_value_throws)
{
	BOOST_CHECK_EXCEPTION(
		[]()
	{
		throws_on_copy_construction const original;
		Si::error_or<throws_on_copy_construction> e((original));
	}(),
		expected_exception,
		[](expected_exception const &)
	{
		return true;
	});
}

BOOST_AUTO_TEST_CASE(error_or_copy_construction_from_error_or_throws)
{
	throws_on_copy_construction original;
	original.active = false;
	Si::error_or<throws_on_copy_construction> e(original);
	e.get_ptr()->active = true;
	BOOST_CHECK_EXCEPTION(
		[&e]()
	{
		Si::error_or<throws_on_copy_construction> f((e));
	}(),
		expected_exception,
		[](expected_exception const &)
	{
		return true;
	});
}

struct throws_on_assignment
{
	throws_on_assignment()
	{
	}

	throws_on_assignment(throws_on_assignment const &)
	{
	}

	throws_on_assignment &operator = (throws_on_assignment const &)
	{
		throw expected_exception();
	}
};

BOOST_AUTO_TEST_CASE(error_or_copy_assignment_from_value_to_success_throws)
{
	Si::error_or<throws_on_assignment> to((throws_on_assignment()));
	BOOST_CHECK_EXCEPTION(
		[&to]()
	{
		throws_on_assignment const original;
		to = original;
	}(),
		expected_exception,
		[](expected_exception const &)
	{
		return true;
	});
}

BOOST_AUTO_TEST_CASE(error_or_copy_assignment_from_value_to_error)
{
	Si::error_or<throws_on_assignment> e(boost::system::error_code(23, boost::system::system_category()));
	throws_on_assignment const original;
	e = original; //should not throw because of the value is copy-constructed
}

BOOST_AUTO_TEST_CASE(error_or_throwing_get)
{
	boost::system::error_code const ec(123, boost::system::native_ecat);
	Si::error_or<int> error(ec);
	BOOST_CHECK(error.is_error());
	BOOST_CHECK_EXCEPTION(error.get(), boost::system::system_error, [ec](boost::system::system_error const &ex)
	{
		return ex.code() == ec;
	});
}

BOOST_AUTO_TEST_CASE(error_or_std)
{
	std::error_code const ec(123, std::system_category());
	Si::error_or<int, std::error_code> error(ec);
	BOOST_CHECK(error.is_error());
	BOOST_CHECK_EXCEPTION(error.get(), std::system_error, [ec](std::system_error const &ex)
	{
		return ex.code() == ec;
	});
}
#endif

struct base
{
	virtual ~base()
	{
	}
};

struct derived : base
{
};

BOOST_AUTO_TEST_CASE(error_or_construct_from_convertible)
{
	Si::error_or<long> e(2u);
	BOOST_CHECK_EQUAL(2L, e.get());
	Si::error_or<std::unique_ptr<base>> f(Si::make_unique<derived>());
	BOOST_CHECK(f.get() != nullptr);
}

BOOST_AUTO_TEST_CASE(error_or_map_value)
{
	BOOST_CHECK_EQUAL(Si::error_or<long>(3L), Si::map(Si::error_or<long>(2L), [](long value)
	{
		return value + 1;
	}));
}

BOOST_AUTO_TEST_CASE(error_or_map_error)
{
	boost::system::error_code const test_error(2, boost::system::native_ecat);
	BOOST_CHECK_EQUAL(
		Si::error_or<long>(test_error),
		Si::map(Si::error_or<long>(test_error), [](long)
	{
		BOOST_FAIL("no value expected");
		return 0;
	}));
}

template <class T>
struct move_only_comparable
{
	BOOST_STATIC_ASSERT(Si::is_nothrow_default_constructible<T>::value);
	BOOST_STATIC_ASSERT(Si::is_nothrow_move_constructible<T>::value);
	BOOST_STATIC_ASSERT(Si::is_nothrow_move_assignable<T>::value);

	T value;

	move_only_comparable() BOOST_NOEXCEPT
	{
	}

	move_only_comparable(T value)
		: value(std::move(value))
	{
	}

	move_only_comparable(move_only_comparable &&other) BOOST_NOEXCEPT
		: value(std::move(other.value))
	{
	}

	move_only_comparable(move_only_comparable const &other)
		: value(other.value)
	{
	}

	move_only_comparable &operator = (move_only_comparable &&other) BOOST_NOEXCEPT
	{
		value = std::move(other.value);
		return *this;
	}

	move_only_comparable &operator = (move_only_comparable const &other)
	{
		value = other.value;
		return *this;
	}
};

template <class T>
bool operator == (move_only_comparable<T> const &left, move_only_comparable<T> const &right)
{
	return left.value == right.value;
}

template <class T>
bool operator != (move_only_comparable<T> const &left, move_only_comparable<T> const &right)
{
	return left.value != right.value;
}

template <class T>
std::ostream &operator << (std::ostream &out, move_only_comparable<T> const &value)
{
	return out << value.value;
}

BOOST_AUTO_TEST_CASE(error_or_equal)
{
	{
		Si::error_or<move_only_comparable<int>> a(2);
		Si::error_or<move_only_comparable<int>> b(2);
		BOOST_CHECK_EQUAL(a, a);
		BOOST_CHECK_EQUAL(a, b);
		BOOST_CHECK_EQUAL(b, a);
		BOOST_CHECK_EQUAL(b, b);
		BOOST_CHECK_EQUAL(a, 2);
		BOOST_CHECK_EQUAL(2, a);
	}

	{
		boost::system::error_code const test_error(2, boost::system::generic_category());
		Si::error_or<move_only_comparable<int>> c = test_error;
		Si::error_or<move_only_comparable<int>> d = test_error;
		BOOST_CHECK_EQUAL(c, c);
		BOOST_CHECK_EQUAL(c, d);
		BOOST_CHECK_EQUAL(d, c);
		BOOST_CHECK_EQUAL(d, d);
		BOOST_CHECK_EQUAL(c, test_error);
		BOOST_CHECK_EQUAL(test_error, c);
	}
}

BOOST_AUTO_TEST_CASE(error_or_not_equal)
{
	Si::error_or<move_only_comparable<int>> a(2);
	Si::error_or<move_only_comparable<int>> b(3);
	Si::error_or<move_only_comparable<int>> c = boost::system::error_code(2, boost::system::generic_category());
	Si::error_or<move_only_comparable<int>> d = boost::system::error_code(3, boost::system::generic_category());

	BOOST_CHECK_NE(a, b);
	BOOST_CHECK_NE(a, c);
	BOOST_CHECK_NE(a, d);

	BOOST_CHECK_NE(b, a);
	BOOST_CHECK_NE(b, c);
	BOOST_CHECK_NE(b, d);

	BOOST_CHECK_NE(c, a);
	BOOST_CHECK_NE(c, b);
	BOOST_CHECK_NE(c, d);

	BOOST_CHECK_NE(d, a);
	BOOST_CHECK_NE(d, b);
	BOOST_CHECK_NE(d, c);

	BOOST_CHECK_NE(a, 3);
	BOOST_CHECK_NE(a, (boost::system::error_code(2 BOOST_PP_COMMA() boost::system::generic_category())));
}

BOOST_AUTO_TEST_CASE(error_or_move_only_in_container)
{
	std::vector<Si::error_or<std::unique_ptr<int>>> v;
	v.emplace_back(Si::make_unique<int>(3));
	v.emplace_back(boost::system::error_code(23, boost::system::system_category()));
	auto w = std::move(v);
	BOOST_CHECK_EQUAL(3, **w[0].get_ptr());
	BOOST_CHECK(boost::system::error_code(23, boost::system::system_category()) == w[1]);
}

BOOST_AUTO_TEST_CASE(error_or_copyable_in_container)
{
	std::vector<Si::error_or<Si::noexcept_string>> v;
	v.emplace_back("Hello");
	v.emplace_back(boost::system::error_code(23, boost::system::system_category()));
	auto w = v;
	BOOST_CHECK(w == v);
	BOOST_CHECK_EQUAL("Hello", w[0].get());
	BOOST_CHECK(boost::system::error_code(23, boost::system::system_category()) == w[1]);
}

namespace boost
{
	// teach Boost.Test how to print std::pair
	template <typename K, typename V>
	wrap_stringstream &operator << (wrap_stringstream &wrapped, std::pair<const K, V> const &item)
	{
		return wrapped << '{' << item.first << ", " << item.second << '}';
	}
}

namespace
{
	template <class Tree, class Hash>
	void test_error_or_as_map_key()
	{
		Tree tree;
		Hash hash;
		for (int i = 0; i < 1000; ++i)
		{
			std::pair<Si::error_or<int>, long> entry(Si::error_or<int>(i), i);
			tree.insert(entry);
			BOOST_CHECK_EQUAL(1u, tree.count(entry.first));
			hash.insert(entry);
			BOOST_CHECK_EQUAL(1u, hash.count(entry.first));
		}
		for (int i = 1000; i < 2000; ++i)
		{
			std::pair<Si::error_or<int>, long> entry(Si::error_or<int>(boost::system::error_code(i, boost::system::system_category())), 1000 + i);
			tree.insert(entry);
			BOOST_CHECK_EQUAL(1u, tree.count(entry.first));
			hash.insert(entry);
			BOOST_CHECK_EQUAL(1u, hash.count(entry.first));
		}
		BOOST_CHECK_EQUAL(2000u, tree.size());
		BOOST_CHECK_EQUAL(2000u, hash.size());
		boost::container::map<Si::error_or<int>, long> tree_from_hash(hash.begin(), hash.end());
		BOOST_CHECK_EQUAL_COLLECTIONS(tree.begin(), tree.end(), tree_from_hash.begin(), tree_from_hash.end());
	}
}

BOOST_AUTO_TEST_CASE(error_or_as_boost_map_key)
{
	test_error_or_as_map_key<
		boost::container::map<Si::error_or<int>, long>,
		boost::unordered_map<Si::error_or<int>, long>
		>();
}

BOOST_AUTO_TEST_CASE(error_or_as_std_map_key)
{
	test_error_or_as_map_key<
		std::map<Si::error_or<int>, long>,
		std::unordered_map<Si::error_or<int>, long>
		>();
}
