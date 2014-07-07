#include <silicium/fast_variant.hpp>
#include <boost/container/string.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/test/unit_test.hpp>

namespace Si
{
#ifdef _MSC_VER
	//boost::container::string is broken in Boost 1.55 with Visual C++ 2013.
	//std::string is not nothrow_default_constructible, but that does not matter because VC++ 2013 cannot detect that anyway.
	typedef std::string noexcept_string;
#else
	typedef boost::container::string noexcept_string;
#endif

	BOOST_AUTO_TEST_CASE(fast_variant_single)
	{
		fast_variant<int> v;
		BOOST_CHECK_EQUAL(0, v.which());
		BOOST_CHECK_EQUAL(boost::make_optional(0), try_get<int>(v));
	}

	BOOST_AUTO_TEST_CASE(fast_variant_assignment_same)
	{
		fast_variant<int, noexcept_string> v(1), w(2);
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
		BOOST_CHECK_EQUAL(boost::make_optional(fast_variant<int>(2)), try_get<fast_variant<int>>(f));
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
		BOOST_CHECK_LT(f, g);
		BOOST_CHECK_LT(f, h);
		BOOST_CHECK_LT(f, i);
		BOOST_CHECK_LT(h, i);
	}
}
