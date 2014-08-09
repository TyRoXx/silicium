#include <silicium/fast_variant.hpp>
#include <reactive/config.hpp>
#include <boost/container/string.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/type_traits/aligned_storage.hpp>

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

	namespace detail
	{
		template <class ...T>
		struct union_;

		template <class First, class ...T>
		struct union_<First, T...>
		{
			union
			{
				First head;
				union_<T...> tail;
			}
			content;
		};

		template <>
		struct union_<>
		{
		};

		template <class T>
		void destroy_storage(void *storage)
		{
			static_cast<T *>(storage)->~T();
		}

		template <class First, class ...Rest>
		struct first
		{
			using type = First;
		};

		template <bool IsCopyable, class ...T>
		struct fast_variant_base;

		template <class ...T>
		struct fast_variant_base<false, T...>
		{
			using which_type = std::size_t;

			fast_variant_base() BOOST_NOEXCEPT
			{
				using constructed = typename first<T...>::type;
				new (reinterpret_cast<constructed *>(&storage)) constructed();
			}

			~fast_variant_base() BOOST_NOEXCEPT
			{
				//hopefully compilers are clever enough to optimize this..
				std::array<void (*)(void *), sizeof...(T)> const f
				{{
					&destroy_storage<T>...
				}};
				f[which_](&storage);
			}

			fast_variant_base(fast_variant_base &&other) BOOST_NOEXCEPT
			{
				throw std::logic_error("to do");
			}

			fast_variant_base &operator = (fast_variant_base &&other) BOOST_NOEXCEPT
			{
				throw std::logic_error("to do");
			}

			template <
				class U,
				class CleanU = typename std::decay<U>::type,
				class NoFastVariant = typename std::enable_if<
					boost::mpl::not_<
						std::is_same<
							CleanU,
							fast_variant<T...>
						>
					>::value,
					void
				>::type>
			fast_variant_base(U &&value)
			{
				throw std::logic_error("to do");
			}

			which_type which() const BOOST_NOEXCEPT
			{
				return which_;
			}

			template <class Visitor>
			auto apply_visitor(Visitor &&visitor) -> typename std::decay<Visitor>::type::result_type;

			template <class Visitor>
			auto apply_visitor(Visitor &&visitor) const -> typename std::decay<Visitor>::type::result_type;

		protected:

			which_type which_ = 0;
			typename boost::aligned_storage<sizeof(union_<T...>)>::type storage;
		};

		template <class ...T>
		struct fast_variant_base<true, T...> : fast_variant_base<false, T...>
		{
			using base = fast_variant_base<false, T...>;

			fast_variant_base() BOOST_NOEXCEPT
			{
			}

			template <
				class U,
				class CleanU = typename std::decay<U>::type,
				class NoFastVariant = typename std::enable_if<
					boost::mpl::not_<
						std::is_same<
							CleanU,
							fast_variant<T...>
						>
					>::value,
					void
				>::type>
			fast_variant_base(U &&value)
				: base(std::forward<U>(value))
			{
			}

			fast_variant_base(fast_variant_base const &other)
			{
				throw std::logic_error("to do");
			}

			fast_variant_base &operator = (fast_variant_base const &other)
			{
				throw std::logic_error("to do");
			}
		};

		template <class ...T>
		struct are_noexcept_movable;

		template <class First, class ...T>
		struct are_noexcept_movable<First, T...>
			: boost::mpl::and_<
				boost::mpl::and_<
					std::is_nothrow_move_constructible<First>,
					std::is_nothrow_move_assignable<First>
				>,
				are_noexcept_movable<T...>
			>::type
		{
		};

		template <>
		struct are_noexcept_movable<> : std::true_type
		{
		};

		template <class ...T>
		struct are_copyable;

		template <class First, class ...T>
		struct are_copyable<First, T...>
			: boost::mpl::and_<
				boost::mpl::and_<
					std::is_copy_constructible<First>,
					std::is_copy_assignable<First>
				>,
				are_copyable<T...>
			>::type
		{
		};

		template <>
		struct are_copyable<> : std::true_type
		{
		};

		BOOST_STATIC_ASSERT(are_copyable<>::value);
		BOOST_STATIC_ASSERT(are_copyable<int>::value);
		BOOST_STATIC_ASSERT(are_copyable<int, float>::value);
		BOOST_STATIC_ASSERT(!are_copyable<int, std::unique_ptr<int>>::value);

		template <class ...T>
		using select_fast_variant_base = fast_variant_base<are_copyable<T...>::value, T...>;
	}

	template <class ...T>
	struct fast_variant2 : detail::select_fast_variant_base<T...>
	{
		BOOST_STATIC_ASSERT_MSG(detail::are_noexcept_movable<T...>::value, "All contained types must be nothrow/noexcept-movable");

		using base = detail::select_fast_variant_base<T...>;

		using base::base;

		fast_variant2()
		{
		}
	};

	template <class ...T>
	bool operator == (fast_variant2<T...> const &left, fast_variant2<T...> const &right)
	{
		if (left.which() != right.which())
		{
			return false;
		}
		throw std::logic_error("to do");
	}

	template <class ...T>
	bool operator != (fast_variant2<T...> const &left, fast_variant2<T...> const &right)
	{
		return !(left == right);
	}

	// default constructor

	BOOST_AUTO_TEST_CASE(fast_variant2_copyable_default)
	{
		using variant = fast_variant2<int>;
		variant v;
		BOOST_CHECK_EQUAL(0, v.which());
	}

	BOOST_AUTO_TEST_CASE(fast_variant2_non_copyable_default)
	{
		using variant = fast_variant2<std::unique_ptr<int>>;
		variant v;
		BOOST_CHECK_EQUAL(0, v.which());
	}

	// move constructor

	BOOST_AUTO_TEST_CASE(fast_variant_non_copyable_construct_move)
	{
		using variant = fast_variant2<std::unique_ptr<int>>;
		variant v(rx::make_unique<int>(2));
		variant w(std::move(v));
		BOOST_CHECK(v != w);
	}

	// copy constructor

	BOOST_AUTO_TEST_CASE(fast_variant2_copyable_construct_copy)
	{
		using variant = fast_variant2<int>;
		variant v;
		variant w(v);
		BOOST_CHECK(v == w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant2_copyable_operator_copy)
	{
		using variant = fast_variant2<int>;
		variant v;
		variant w(2);
		BOOST_CHECK(v != w);
		v = w;
		BOOST_CHECK(v == w);
	}

	// move operator

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_operator_move)
	{
		using variant = fast_variant2<noexcept_string>;
		BOOST_STATIC_ASSERT(std::is_copy_assignable<variant>::value);
		BOOST_STATIC_ASSERT(std::is_copy_constructible<variant>::value);
		variant v;
		variant w(noexcept_string(1000, 'a'));
		BOOST_CHECK(v != w);
		v = std::move(w);
		BOOST_CHECK(v != w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_non_copyable_operator_move)
	{
		using variant = fast_variant2<std::unique_ptr<int>>;
		variant v;
		variant w(rx::make_unique<int>(2));
		BOOST_CHECK(v != w);
		v = std::move(w);
		BOOST_CHECK(v != w);
	}

	// copy operator

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_operator_copy_to_same)
	{
		using variant = fast_variant2<int, double>;
		variant v;
		variant w(3);
		BOOST_CHECK(v != w);
		v = w;
		BOOST_CHECK(v == w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_operator_copy_to_different)
	{
		using variant = fast_variant2<int, double>;
		variant v(1.0);
		variant w(3);
		BOOST_CHECK(v != w);
		v = w;
		BOOST_CHECK(v == w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_operator_copy_raw)
	{
		using variant = fast_variant2<int, double>;
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
}
