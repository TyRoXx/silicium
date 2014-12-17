#include <silicium/fast_variant.hpp>
#include <silicium/config.hpp>
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

	BOOST_AUTO_TEST_CASE(fast_variant_single)
	{
		fast_variant<int> v;
		BOOST_CHECK_EQUAL(0U, v.which());
		BOOST_CHECK_EQUAL(boost::make_optional(0), try_get<int>(v));
	}

	BOOST_AUTO_TEST_CASE(fast_variant_assignment_same)
	{
		typedef fast_variant<int, noexcept_string> variant;
		variant v(1), w(2);
		BOOST_CHECK_EQUAL(boost::make_optional(1), try_get<int>(v));
		BOOST_CHECK_EQUAL(boost::make_optional(2), try_get<int>(w));
		v = w;
		BOOST_CHECK_EQUAL(boost::make_optional(2), try_get<int>(v));
		BOOST_CHECK_EQUAL(boost::make_optional(2), try_get<int>(w));
	}

	BOOST_AUTO_TEST_CASE(fast_variant_assignment_different)
	{
		fast_variant<int, noexcept_string> v, w(noexcept_string("S"));
		BOOST_CHECK_EQUAL(boost::make_optional(0), try_get<int>(v));
		BOOST_CHECK_EQUAL(boost::make_optional(noexcept_string("S")), try_get<noexcept_string>(w));
		v = w;
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

	BOOST_AUTO_TEST_CASE(fast_variant_apply_visitor)
	{
		{
			fast_variant<int, noexcept_string> int_;
			is_int_visitor v;
			bool is_int = apply_visitor(v, int_);
			BOOST_CHECK(is_int);
		}

		{
			fast_variant<int, noexcept_string> str(noexcept_string{});
			is_int_visitor v;
			bool is_int = apply_visitor(v, str);
			BOOST_CHECK(!is_int);
		}
	}

	BOOST_AUTO_TEST_CASE(fast_variant_nesting)
	{
		fast_variant<fast_variant<int>, noexcept_string> f;
		f = noexcept_string("S");
		BOOST_CHECK_EQUAL(boost::make_optional(noexcept_string("S")), try_get<noexcept_string>(f));

		f = fast_variant<int>(2);
		BOOST_CHECK(boost::make_optional(fast_variant<int>(2)) == try_get<fast_variant<int>>(f));
	}

	BOOST_AUTO_TEST_CASE(fast_variant_construct_const)
	{
		noexcept_string const s("S");
		fast_variant<fast_variant<int>, noexcept_string> f{s};
	}

	BOOST_AUTO_TEST_CASE(fast_variant_assign_const)
	{
		fast_variant<fast_variant<int>, noexcept_string> f;
		noexcept_string const s("S");
		f = s;
	}

	BOOST_AUTO_TEST_CASE(fast_variant_less)
	{
		fast_variant<int, noexcept_string> f(1), g(2), h(noexcept_string("a")), i(noexcept_string("b"));
		BOOST_CHECK(f < g);
		BOOST_CHECK(f < h);
		BOOST_CHECK(f < i);
		BOOST_CHECK(h < i);
	}

	// default constructor

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_default)
	{
		typedef fast_variant<int> variant;
		variant v;
		BOOST_CHECK_EQUAL(0U, v.which());
	}

	BOOST_AUTO_TEST_CASE(fast_variant_non_copyable_default)
	{
		typedef fast_variant<std::unique_ptr<int>> variant;
		variant v;
		BOOST_CHECK_EQUAL(0U, v.which());
	}

	// move constructor

	BOOST_AUTO_TEST_CASE(fast_variant_non_copyable_construct_move)
	{
		typedef fast_variant<std::unique_ptr<int>> variant;
		variant v(Si::make_unique<int>(2));
		variant w(std::move(v));
		BOOST_CHECK(v != w);
	}

	// copy constructor

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_construct_copy)
	{
		typedef fast_variant<int> variant;
		variant v;
		variant w(v);
		BOOST_CHECK(v == w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_operator_copy)
	{
		typedef fast_variant<int> variant;
		variant v;
		variant w(2);
		BOOST_CHECK(v != w);
		v = w;
		BOOST_CHECK(v == w);
	}

	// move operator

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_operator_move)
	{
		typedef fast_variant<noexcept_string> variant;
		BOOST_STATIC_ASSERT(Si::is_copy_assignable<variant>::value);
		BOOST_STATIC_ASSERT(Si::is_copy_constructible<variant>::value);
		variant v;
		variant w(noexcept_string(1000, 'a'));
		BOOST_CHECK(v != w);
		v = std::move(w);
		BOOST_CHECK(v != w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_non_copyable_operator_move)
	{
		typedef fast_variant<std::unique_ptr<int>> variant;
		variant v;
		variant w(Si::make_unique<int>(2));
		BOOST_CHECK(v != w);
		v = std::move(w);
		BOOST_CHECK(v != w);
	}

	// copy operator

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_operator_copy_to_same)
	{
		typedef fast_variant<int, double> variant;
		variant v;
		variant w(3);
		BOOST_CHECK(v != w);
		v = w;
		BOOST_CHECK(v == w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_operator_copy_to_different)
	{
		typedef fast_variant<int, double> variant;
		variant v(1.0);
		variant w(3);
		BOOST_CHECK(v != w);
		v = w;
		BOOST_CHECK(v == w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_operator_copy_raw)
	{
		typedef fast_variant<int, double> variant;
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

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_apply_visitor_mutable)
	{
		typedef fast_variant<int, double> variant;
		variant v(2);
		bool success = apply_visitor(test_visitor_1(), v);
		BOOST_CHECK(success);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_apply_visitor_const)
	{
		typedef fast_variant<int, double> variant;
		variant const v(2);
		bool success = apply_visitor(test_visitor_1(), v);
		BOOST_CHECK(success);
	}

	// comparison operators

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_equal)
	{
		fast_variant<int, float> v, w;
		BOOST_CHECK(v == w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_not_equal)
	{
		fast_variant<int, float> v, w, x(2), y(2.0f);
		BOOST_CHECK(!(v != w));
		BOOST_CHECK(v != x);
		BOOST_CHECK(x != y);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_less_which)
	{
		fast_variant<int, float> v(1), w(1.0f);
		BOOST_CHECK(v < w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_less_content)
	{
		fast_variant<int, float> v(1), w(2);
		BOOST_CHECK(v < w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_less_equal_which)
	{
		fast_variant<int, float> v(1), w(1.0f);
		BOOST_CHECK(v <= w);
		BOOST_CHECK(v <= v);
		BOOST_CHECK(w <= w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_less_equal_content)
	{
		fast_variant<int, float> v(1), w(2);
		BOOST_CHECK(v <= w);
		BOOST_CHECK(v <= v);
		BOOST_CHECK(w <= w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_greater_which)
	{
		fast_variant<int, float> v(1), w(1.0f);
		BOOST_CHECK(w > v);
		BOOST_CHECK(!(v > v));
		BOOST_CHECK(!(w > w));
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_greater_content)
	{
		fast_variant<int, float> v(1), w(2);
		BOOST_CHECK(w > v);
		BOOST_CHECK(!(w > w));
		BOOST_CHECK(!(v > v));
		BOOST_CHECK(!(w > w));
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_greater_equal_which)
	{
		fast_variant<int, float> v(1), w(1.0f);
		BOOST_CHECK(w >= v);
		BOOST_CHECK(v >= v);
		BOOST_CHECK(w >= w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_greater_equal_content)
	{
		fast_variant<int, float> v(1), w(2);
		BOOST_CHECK(w >= v);
		BOOST_CHECK(v >= v);
		BOOST_CHECK(w >= w);
	}

	// std::hash

	BOOST_AUTO_TEST_CASE(fast_variant_hash)
	{
		typedef fast_variant<int, float> variant;
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

	BOOST_AUTO_TEST_CASE(fast_variant_const_visit)
	{
		typedef fast_variant<int, float> variant;
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

	BOOST_AUTO_TEST_CASE(fast_variant_mutable_visit)
	{
		typedef fast_variant<int, float> variant;
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

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_mutable_assign_subset)
	{
		fast_variant<int> v(2);
		fast_variant<float, int> w;
		w.assign(v);
		BOOST_CHECK_EQUAL(boost::make_optional<int>(2), try_get<int>(w));
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_const_assign_subset)
	{
		fast_variant<int> const v(2);
		fast_variant<float, int> w;
		w.assign(v);
		BOOST_CHECK_EQUAL(boost::make_optional<int>(2), try_get<int>(w));
	}

#if 0 //TODO: make this work
	BOOST_AUTO_TEST_CASE(fast_variant_non_copyable_construct_superset)
	{
		fast_variant<std::unique_ptr<int>> v(Si::make_unique<int>(2));
		fast_variant<float, std::unique_ptr<int>> w;
		w.assign(std::move(v)); //would not compile because assign uses apply_visitor which does not forward rvalue-ness yet
		auto * const element = try_get_ptr<std::unique_ptr<int>>(w);
		BOOST_REQUIRE(element);
		BOOST_REQUIRE(*element);
		BOOST_CHECK_EQUAL(2, **element);
	}
#endif

	namespace detail
	{
		template <class Method>
		struct argument_of_method;

		template <class Result, class Class, class Argument>
		struct argument_of_method<Result(Class::*)(Argument) const>
		{
			typedef Argument type;
		};

		template <class Result, class Class, class Argument>
		struct argument_of_method<Result(Class::*)(Argument)>
		{
			typedef Argument type;
		};

		template <class Function>
		struct argument_of : argument_of_method<decltype(&Function::operator())>
		{
		};

		template <class Result, class Argument>
		struct argument_of<Result(*)(Argument)>
		{
			typedef Argument type;
		};

		BOOST_STATIC_ASSERT(std::is_same<int, argument_of<void(*)(int)>::type>::value);

		template <class Result, class ...Functions>
		struct overloader;

		template <class Result, class Head>
		struct overloader<Result, Head>
		{
			//for apply_visitor
			typedef Result result_type;

			explicit overloader(Head &head)
				: m_head(&head)
			{
			}

			Result operator()(typename argument_of<Head>::type argument) const
			{
				return (*m_head)(std::forward<typename argument_of<Head>::type>(argument));
			}

		private:

			Head *m_head;
		};

		template <class Result, class Head, class ...Tail>
		struct overloader<Result, Head, Tail...> : overloader<Result, Tail...>
		{
			using overloader<Result, Tail...>::operator();

			//for apply_visitor
			typedef Result result_type;

			explicit overloader(Head &head, Tail &...tail)
				: overloader<Result, Tail...>(tail...)
				, m_head(&head)
			{
			}

			Result operator()(typename argument_of<Head>::type argument) const
			{
				return (*m_head)(std::forward<typename argument_of<Head>::type>(argument));
			}

		private:

			Head *m_head;
		};
	}

	template <class Result, class ...T, class ...Visitors>
	Result overloaded_visit(fast_variant<T...> &variant, Visitors &&...visitors)
	{
		detail::overloader<Result, Visitors...> ov(visitors...);
		return Si::apply_visitor(ov, variant);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_overloaded_visit_const_visitors)
	{
		fast_variant<int, nothing, std::unique_ptr<long>> v(3);
		int result = overloaded_visit<int>(
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

	BOOST_AUTO_TEST_CASE(fast_variant_overloaded_visit_mutable_visitors)
	{
		fast_variant<int, nothing, std::unique_ptr<long>> v(3);
		int result = overloaded_visit<int>(
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
}
