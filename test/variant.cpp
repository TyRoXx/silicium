#include <silicium/variant.hpp>
#include <silicium/config.hpp>
#include <silicium/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/container/string.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/test/unit_test.hpp>
#include <unordered_set>

namespace Si
{
#ifdef _MSC_VER
	//boost::container::string is broken in Boost 1.55 with Visual C++ 2013.
	//std::string is not nothrow_default_constructible, but that does not matter because VC++ 2013 cannot detect that anyway.
	typedef std::string noexcept_string;
#elif BOOST_VERSION >= 105300
	typedef boost::container::string noexcept_string;
#else
	typedef char const *noexcept_string;
#endif

	BOOST_AUTO_TEST_CASE(variant_single)
	{
		variant<int> v;
		BOOST_CHECK_EQUAL(0U, v.which());
		BOOST_CHECK_EQUAL(boost::make_optional(0), try_get<int>(v));
	}

	BOOST_AUTO_TEST_CASE(variant_assignment_same)
	{
		typedef variant<int, noexcept_string> variant;
		variant v(1), w(2);
		BOOST_CHECK_EQUAL(boost::make_optional(1), try_get<int>(v));
		BOOST_CHECK_EQUAL(boost::make_optional(2), try_get<int>(w));
		v = w;
		BOOST_CHECK_EQUAL(v.which(), w.which());
		BOOST_CHECK_EQUAL(v.which(), 0u);
		BOOST_CHECK_EQUAL(boost::make_optional(2), try_get<int>(v));
		BOOST_CHECK_EQUAL(boost::make_optional(2), try_get<int>(w));
	}

	BOOST_AUTO_TEST_CASE(variant_assignment_different)
	{
		variant<int, noexcept_string> v, w(noexcept_string("S"));
		BOOST_CHECK_EQUAL(boost::make_optional(0), try_get<int>(v));
		BOOST_CHECK_EQUAL(boost::make_optional(noexcept_string("S")), try_get<noexcept_string>(w));
		v = w;
		BOOST_CHECK_EQUAL(v.which(), w.which());
		BOOST_CHECK_EQUAL(v.which(), 1u);
		BOOST_CHECK_EQUAL(boost::make_optional(noexcept_string("S")), try_get<noexcept_string>(v));
		BOOST_CHECK_EQUAL(boost::make_optional(noexcept_string("S")), try_get<noexcept_string>(w));
	}

	struct is_int_visitor : boost::static_visitor<bool>
	{
		bool operator()(int) const
		{
			return true;
		}

		bool operator()(noexcept_string const &) const
		{
			return false;
		}
	};

	BOOST_AUTO_TEST_CASE(variant_apply_visitor)
	{
		{
			variant<int, noexcept_string> int_;
			is_int_visitor v;
			bool is_int = apply_visitor(v, int_);
			BOOST_CHECK(is_int);
		}

		{
			variant<int, noexcept_string> str(noexcept_string{});
			is_int_visitor v;
			bool is_int = apply_visitor(v, str);
			BOOST_CHECK(!is_int);
		}
	}

	BOOST_AUTO_TEST_CASE(variant_nesting)
	{
		variant<variant<int>, noexcept_string> f;
		f = noexcept_string("S");
		BOOST_CHECK_EQUAL(boost::make_optional(noexcept_string("S")), try_get<noexcept_string>(f));

		f = variant<int>(2);
		BOOST_CHECK(boost::make_optional(variant<int>(2)) == try_get<variant<int>>(f));
	}

	BOOST_AUTO_TEST_CASE(variant_construct_const)
	{
		noexcept_string const s("S");
		variant<variant<int>, noexcept_string> f{s};
	}

	BOOST_AUTO_TEST_CASE(variant_assign_const)
	{
		variant<variant<int>, noexcept_string> f;
		noexcept_string const s("S");
		f = s;
	}

	BOOST_AUTO_TEST_CASE(variant_less)
	{
		variant<int, noexcept_string> f(1), g(2), h(noexcept_string("a")), i(noexcept_string("b"));
		BOOST_CHECK(f < g);
		BOOST_CHECK(f < h);
		BOOST_CHECK(f < i);
		BOOST_CHECK(h < i);
	}

	// default constructor

	BOOST_AUTO_TEST_CASE(variant_copyable_default)
	{
		typedef variant<int> variant;
		variant v;
		BOOST_CHECK_EQUAL(0U, v.which());
	}

	BOOST_AUTO_TEST_CASE(variant_non_copyable_default)
	{
		typedef variant<std::unique_ptr<int>> variant;
		variant v;
		BOOST_CHECK_EQUAL(0U, v.which());
	}

	// move constructor

	BOOST_AUTO_TEST_CASE(variant_non_copyable_construct_move)
	{
		typedef variant<std::unique_ptr<int>> variant;
		variant v(Si::make_unique<int>(2));
		variant w(std::move(v));
		BOOST_CHECK(v != w);
	}

	// copy constructor

	BOOST_AUTO_TEST_CASE(variant_copyable_construct_copy)
	{
		typedef variant<int> variant;
		variant v;
		variant w(v);
		BOOST_CHECK(v == w);
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_operator_copy)
	{
		typedef variant<int> variant;
		variant v;
		variant w(2);
		BOOST_CHECK(v != w);
		v = w;
		BOOST_CHECK(v == w);
	}

	// move operator

	BOOST_AUTO_TEST_CASE(variant_copyable_operator_move)
	{
		typedef variant<noexcept_string> variant;
		BOOST_STATIC_ASSERT(Si::is_copy_assignable<variant>::value);
		BOOST_STATIC_ASSERT(Si::is_copy_constructible<variant>::value);
		variant v;
		variant w(noexcept_string(1000, 'a'));
		BOOST_CHECK(v != w);
		v = std::move(w);
		BOOST_CHECK(v != w);
	}

	BOOST_AUTO_TEST_CASE(variant_non_copyable_operator_move)
	{
		typedef variant<std::unique_ptr<int>> variant;
		variant v;
		variant w(Si::make_unique<int>(2));
		BOOST_CHECK(v != w);
		v = std::move(w);
		BOOST_CHECK(v != w);
	}

	// copy operator

	BOOST_AUTO_TEST_CASE(variant_copyable_operator_copy_to_same)
	{
		typedef variant<int, double> variant;
		variant v;
		variant w(3);
		BOOST_CHECK(v != w);
		v = w;
		BOOST_CHECK(v == w);
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_operator_copy_to_different)
	{
		typedef variant<int, double> variant;
		variant v(1.0);
		variant w(3);
		BOOST_CHECK(v != w);
		v = w;
		BOOST_CHECK(v == w);
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_operator_copy_raw)
	{
		typedef variant<int, double> variant;
		variant v;
		variant w(3);
		BOOST_CHECK(v != w);
		v = 3;
		BOOST_CHECK(v == w);
		w = 2.0;
		BOOST_CHECK(v != w);
		v = 2.0;
		BOOST_CHECK(v == w);
	}

	// apply_visitor

	struct test_visitor_1 : boost::static_visitor<bool>
	{
		bool operator()(int i) const
		{
			return (i == 2);
		}

		bool operator()(double) const
		{
			return false;
		}
	};

	BOOST_AUTO_TEST_CASE(variant_copyable_apply_visitor_mutable)
	{
		typedef variant<int, double> variant;
		variant v(2);
		bool success = apply_visitor(test_visitor_1(), v);
		BOOST_CHECK(success);
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_apply_visitor_const)
	{
		typedef variant<int, double> variant;
		variant const v(2);
		bool success = apply_visitor(test_visitor_1(), v);
		BOOST_CHECK(success);
	}

	// comparison operators

	BOOST_AUTO_TEST_CASE(variant_copyable_equal)
	{
		variant<int, float> v, w;
		BOOST_CHECK(v == w);
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_not_equal)
	{
		variant<int, float> v, w, x(2), y(2.0f);
		BOOST_CHECK(!(v != w));
		BOOST_CHECK(v != x);
		BOOST_CHECK(x != y);
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_less_which)
	{
		variant<int, float> v(1), w(1.0f);
		BOOST_CHECK(v < w);
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_less_content)
	{
		variant<int, float> v(1), w(2);
		BOOST_CHECK(v < w);
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_less_equal_which)
	{
		variant<int, float> v(1), w(1.0f);
		BOOST_CHECK(v <= w);
		BOOST_CHECK(v <= v);
		BOOST_CHECK(w <= w);
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_less_equal_content)
	{
		variant<int, float> v(1), w(2);
		BOOST_CHECK(v <= w);
		BOOST_CHECK(v <= v);
		BOOST_CHECK(w <= w);
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_greater_which)
	{
		variant<int, float> v(1), w(1.0f);
		BOOST_CHECK(w > v);
		BOOST_CHECK(!(v > v));
		BOOST_CHECK(!(w > w));
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_greater_content)
	{
		variant<int, float> v(1), w(2);
		BOOST_CHECK(w > v);
		BOOST_CHECK(!(w > w));
		BOOST_CHECK(!(v > v));
		BOOST_CHECK(!(w > w));
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_greater_equal_which)
	{
		variant<int, float> v(1), w(1.0f);
		BOOST_CHECK(w >= v);
		BOOST_CHECK(v >= v);
		BOOST_CHECK(w >= w);
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_greater_equal_content)
	{
		variant<int, float> v(1), w(2);
		BOOST_CHECK(w >= v);
		BOOST_CHECK(v >= v);
		BOOST_CHECK(w >= w);
	}

	// std::hash

	BOOST_AUTO_TEST_CASE(variant_hash)
	{
		typedef variant<int, float> variant;
		std::unordered_set<variant> s;
		s.insert(2);
		BOOST_CHECK_EQUAL(1U, s.count(2));
		s.insert(3);
		BOOST_CHECK_EQUAL(1U, s.count(3));
		s.erase(2);
		BOOST_CHECK_EQUAL(0U, s.count(2));
		BOOST_CHECK_EQUAL(1U, s.count(3));
		s.insert(2.0f);
		BOOST_CHECK_EQUAL(0U, s.count(2));
		BOOST_CHECK_EQUAL(1U, s.count(2.0f));
	}

	// visit

	BOOST_AUTO_TEST_CASE(variant_const_visit)
	{
		typedef variant<int, float> variant;
		variant const v(2);
		BOOST_CHECK_EQUAL(2 + 1, visit<int>(
			v,
			[](int i) -> int
		{
			return i + 1;
		},
			[](float) -> int
		{
			BOOST_REQUIRE(false);
			return 0;
		}));
	}

	BOOST_AUTO_TEST_CASE(variant_mutable_visit)
	{
		typedef variant<int, float> variant;
		variant v(2);
		BOOST_CHECK_EQUAL(2 + 1, visit<int>(
			v,
			[](int i) -> int
		{
			return i + 1;
		},
			[](float) -> int
		{
			BOOST_REQUIRE(false);
			return 0;
		}));
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_mutable_assign_subset)
	{
		variant<int> v(2);
		variant<float, int> w;
		w.assign(v);
		BOOST_CHECK_EQUAL(boost::make_optional<int>(2), try_get<int>(w));
	}

	BOOST_AUTO_TEST_CASE(variant_copyable_const_assign_subset)
	{
		variant<int> const v(2);
		variant<float, int> w;
		w.assign(v);
		BOOST_CHECK_EQUAL(boost::make_optional<int>(2), try_get<int>(w));
	}

#if 0 //TODO: make this work
	BOOST_AUTO_TEST_CASE(variant_non_copyable_construct_superset)
	{
		variant<std::unique_ptr<int>> v(Si::make_unique<int>(2));
		variant<float, std::unique_ptr<int>> w;
		w.assign(std::move(v)); //would not compile because assign uses apply_visitor which does not forward rvalue-ness yet
		auto * const element = try_get_ptr<std::unique_ptr<int>>(w);
		BOOST_REQUIRE(element);
		BOOST_REQUIRE(*element);
		BOOST_CHECK_EQUAL(2, **element);
	}
#endif

	BOOST_AUTO_TEST_CASE(variant_overloaded_visit_unordered_with_const_visitors)
	{
		variant<int, nothing, std::unique_ptr<long>> v(3);
		int result = visit<int>(
			v,
			[](nothing) -> int
			{
				BOOST_FAIL("unexpected type");
				return 0;
			},
			[](int element) -> int
			{
				return element;
			},
			[](std::unique_ptr<long> &) -> int
			{
				BOOST_FAIL("unexpected type");
				return 0;
			}
		);
		BOOST_CHECK_EQUAL(3, result);
	}

	BOOST_AUTO_TEST_CASE(variant_visit_unordered_with_mutable_visitors)
	{
		variant<int, nothing, std::unique_ptr<long>> v(3);
		int result = visit<int>(
			v,
			[](nothing) mutable -> int
			{
				BOOST_FAIL("unexpected type");
				return 0;
			},
			[](int element) mutable -> int
			{
				return element;
			},
			[](std::unique_ptr<long> &) mutable -> int
			{
				BOOST_FAIL("unexpected type");
				return 0;
			}
		);
		BOOST_CHECK_EQUAL(3, result);
	}

	BOOST_AUTO_TEST_CASE(variant_visit_return_void)
	{
		variant<int, nothing> v(3);
		bool got_result = false;
		visit<void>(
			v,
			[](nothing)
			{
				BOOST_FAIL("unexpected type");
			},
			[&got_result](int element)
			{
				BOOST_REQUIRE(!got_result);
				got_result = true;
				BOOST_CHECK_EQUAL(3, element);
			}
		);
		BOOST_CHECK(got_result);
	}

	struct needs_inplace_construction : boost::noncopyable
	{
		int v;

		explicit needs_inplace_construction(int v)
			: v(v)
		{
		}
	};

	bool operator == (needs_inplace_construction const &left, needs_inplace_construction const &right)
	{
		return left.v == right.v;
	}

	std::ostream &operator << (std::ostream &out, needs_inplace_construction const &value)
	{
		return out << value.v;
	}

	std::ostream &operator << (std::ostream &out, Si::nothing)
	{
		return out << "nothing";
	}

	BOOST_AUTO_TEST_CASE(variant_construct_inplace)
	{
		typedef variant<int, float, nothing, std::string, needs_inplace_construction> var;
		{
			var a(Si::inplace<int>(), 12);
			var b(12);
			BOOST_CHECK_EQUAL(b, a);
		}
		{
			var a(Si::inplace<float>(), 12.0f);
			var b(12.0f);
			BOOST_CHECK_EQUAL(b, a);
		}
		{
			var a((Si::inplace<nothing>()), Si::nothing());
			var b((Si::nothing()));
			BOOST_CHECK_EQUAL(b, a);
		}
		{
			var a((Si::inplace<std::string>()), "test");
			var b(std::string("test"));
			BOOST_CHECK_EQUAL(b, a);
		}
		{
			var a((Si::inplace<needs_inplace_construction>()), 123);
			BOOST_CHECK_EQUAL(a, a);
			BOOST_CHECK_EQUAL(123, Si::visit<optional<int>>(
				a,
				[](int) { BOOST_FAIL("unexpected type"); return none; },
				[](float) { BOOST_FAIL("unexpected type"); return none; },
				[](nothing) { BOOST_FAIL("unexpected type"); return none; },
				[](std::string const &) { BOOST_FAIL("unexpected type"); return none; },
				[](needs_inplace_construction const &value) { return value.v; }
			));
		}
	}
}