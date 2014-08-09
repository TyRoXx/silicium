#include <silicium/fast_variant.hpp>
#include <reactive/config.hpp>
#include <boost/container/string.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <unordered_set>

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
		void destroy_storage(void *storage) BOOST_NOEXCEPT
		{
			static_cast<T *>(storage)->~T();
		}

		template <class T>
		void move_construct_storage(void *destination, void *source) BOOST_NOEXCEPT
		{
			auto &dest = *static_cast<T *>(destination);
			auto &src = *static_cast<T *>(source);
			new (&dest) T(std::move(src));
		}

		template <class T>
		void copy_construct_storage(void *destination, void const *source)
		{
			auto &dest = *static_cast<T *>(destination);
			auto &src = *static_cast<T const *>(source);
			new (&dest) T(src);
		}

		template <class T>
		void move_storage(void *destination, void *source) BOOST_NOEXCEPT
		{
			auto &dest = *static_cast<T *>(destination);
			auto &src = *static_cast<T *>(source);
			dest = std::move(src);
		}

		template <class T>
		void copy_storage(void *destination, void const *source)
		{
			auto &dest = *static_cast<T *>(destination);
			auto &src = *static_cast<T const *>(source);
			dest = src;
		}

		template <class T>
		bool equals(void const *left, void const *right)
		{
			auto &left_ = *static_cast<T const *>(left);
			auto &right_ = *static_cast<T const *>(right);
			return left_ == right_;
		}

		template <class T>
		bool less(void const *left, void const *right)
		{
			auto &left_ = *static_cast<T const *>(left);
			auto &right_ = *static_cast<T const *>(right);
			return left_ < right_;
		}

		template <class Visitor, class T, class Result>
		Result apply_visitor_impl(Visitor &&visitor, void *element)
		{
			return std::forward<Visitor>(visitor)(*static_cast<T *>(element));
		}

		template <class Visitor, class T, class Result>
		Result apply_visitor_const_impl(Visitor &&visitor, void const *element)
		{
			return std::forward<Visitor>(visitor)(*static_cast<T const *>(element));
		}

		template <class First, class ...Rest>
		struct first
		{
			using type = First;
		};

		template <class Element, class ...All>
		struct index_of : boost::mpl::find<boost::mpl::vector<All...>, Element>::type::pos
		{
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
				destroy_storage(which_, storage);
			}

			fast_variant_base(fast_variant_base &&other) BOOST_NOEXCEPT
				: which_(other.which_)
			{
				move_construct_storage(which_, storage, other.storage);
			}

			fast_variant_base &operator = (fast_variant_base &&other) BOOST_NOEXCEPT
			{
				if (which_ == other.which_)
				{
					move_storage(which_, storage, other.storage);
				}
				else
				{
					destroy_storage(which_, storage);
					move_construct_storage(other.which_, storage, other.storage);
					which_ = other.which_;
				}
				return *this;
			}

			template <
				class U,
				class CleanU = typename std::decay<U>::type,
				std::size_t Index = index_of<CleanU, T...>::value,
				class NoFastVariant = typename std::enable_if<
					boost::mpl::and_<
						boost::mpl::not_<
							std::is_base_of<
								fast_variant_base,
								CleanU
							>
						>,
						boost::mpl::bool_<Index < sizeof...(T)>
					>::value,
					void
				>::type>
			fast_variant_base(U &&value)
				: which_(Index)
			{
				new (&storage) CleanU(std::forward<U>(value));
			}

			which_type which() const BOOST_NOEXCEPT
			{
				return which_;
			}

			template <class Visitor>
			auto apply_visitor(Visitor &&visitor) -> typename std::decay<Visitor>::type::result_type
			{
				using result = typename std::decay<Visitor>::type::result_type;
				std::array<result (*)(Visitor &&, void *), sizeof...(T)> const f
				{{
					&apply_visitor_impl<Visitor, T, result>...
				}};
				return f[which_](std::forward<Visitor>(visitor), &storage);
			}

			template <class Visitor>
			auto apply_visitor(Visitor &&visitor) const -> typename std::decay<Visitor>::type::result_type
			{
				using result = typename std::decay<Visitor>::type::result_type;
				std::array<result (*)(Visitor &&, void const *), sizeof...(T)> const f
				{{
					&apply_visitor_const_impl<Visitor, T, result>...
				}};
				return f[which_](std::forward<Visitor>(visitor), &storage);
			}

			bool operator == (fast_variant_base const &other) const
			{
				if (which_ != other.which_)
				{
					return false;
				}
				std::array<bool (*)(void const *, void const *), sizeof...(T)> const f =
				{{
					&equals<T>...
				}};
				return f[which_](&storage, &other.storage);
			}

			bool operator < (fast_variant_base const &other) const
			{
				if (which_ > other.which_)
				{
					return false;
				}
				if (which_ < other.which_)
				{
					return true;
				}
				std::array<bool (*)(void const *, void const *), sizeof...(T)> const f =
				{{
					&less<T>...
				}};
				return f[which_](&storage, &other.storage);
			}

		protected: //TODO: make private somehow

			using storage_type = typename boost::aligned_storage<sizeof(union_<T...>)>::type;

			which_type which_ = 0;
			storage_type storage;

			static void copy_construct_storage(which_type which, storage_type &destination, storage_type const &source)
			{
				std::array<void (*)(void *, void const *), sizeof...(T)> const f =
				{{
					&detail::copy_construct_storage<T>...
				}};
				f[which](&destination, &source);
			}

			static void move_construct_storage(which_type which, storage_type &destination, storage_type &source) BOOST_NOEXCEPT
			{
				std::array<void (*)(void *, void *), sizeof...(T)> const f =
				{{
					&detail::move_construct_storage<T>...
				}};
				f[which](&destination, &source);
			}

			static void destroy_storage(which_type which, storage_type &destroyed) BOOST_NOEXCEPT
			{
				std::array<void (*)(void *), sizeof...(T)> const f =
				{{
					&detail::destroy_storage<T>...
				}};
				f[which](&destroyed);
			}

			static void copy_storage(which_type which, storage_type &destination, storage_type const &source)
			{
				std::array<void (*)(void *, void const *), sizeof...(T)> const f =
				{{
					&detail::copy_storage<T>...
				}};
				f[which](&destination, &source);
			}

			static void move_storage(which_type which, storage_type &destination, storage_type &source) BOOST_NOEXCEPT
			{
				std::array<void (*)(void *, void *), sizeof...(T)> const f =
				{{
					&detail::move_storage<T>...
				}};
				f[which](&destination, &source);
			}
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
				std::size_t Index = index_of<CleanU, T...>::value,
				class NoFastVariant = typename std::enable_if<
					boost::mpl::and_<
						boost::mpl::not_<
							std::is_base_of<
								fast_variant_base,
								CleanU
							>
						>,
						boost::mpl::bool_<Index < sizeof...(T)>
					>::value,
					void
				>::type>
			fast_variant_base(U &&value)
				: base(std::forward<U>(value))
			{
			}

			fast_variant_base(fast_variant_base &&other) BOOST_NOEXCEPT
				: base(std::move(other))
			{
			}

			fast_variant_base(fast_variant_base const &other)
				: base()
			{
				this->which_ = other.which();
				base::copy_construct_storage(this->which_, this->storage, other.storage);
			}

			fast_variant_base &operator = (fast_variant_base &&other) BOOST_NOEXCEPT
			{
				base::operator = (std::move(other));
				return *this;
			}

			fast_variant_base &operator = (fast_variant_base const &other)
			{
				if (this->which_ == other.which_)
				{
					base::copy_storage(this->which_, this->storage, other.storage);
				}
				else
				{
					typename base::storage_type temporary;
					base::copy_construct_storage(other.which_, temporary, other.storage);
					base::destroy_storage(this->which_, this->storage);
					base::move_construct_storage(other.which_, this->storage, temporary);
					this->which_ = other.which_;
				}
				return *this;
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
	bool operator != (fast_variant2<T...> const &left, fast_variant2<T...> const &right)
	{
		return !(left == right);
	}

	struct hash_visitor : boost::static_visitor<std::size_t>
	{
		template <class T>
		std::size_t operator()(T const &element) const
		{
			return std::hash<T>()(element);
		}
	};
}

namespace std
{
	template <class ...T>
	struct hash<Si::fast_variant2<T...>>
	{
		std::size_t operator()(Si::fast_variant2<T...> const &v) const
		{
			return Si::apply_visitor(Si::hash_visitor(), v);
		}
	};
}

namespace Si
{
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
		using variant = fast_variant2<int, double>;
		variant v(2);
		bool success = Si::apply_visitor(test_visitor_1(), v);
		BOOST_CHECK(success);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_apply_visitor_const)
	{
		using variant = fast_variant2<int, double>;
		variant const v(2);
		bool success = Si::apply_visitor(test_visitor_1(), v);
		BOOST_CHECK(success);
	}

	// comparison operators

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_equal)
	{
		fast_variant2<int, float> v, w;
		BOOST_CHECK(v == w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_less_which)
	{
		fast_variant2<int, float> v(1), w(1.0f);
		BOOST_CHECK(v < w);
	}

	BOOST_AUTO_TEST_CASE(fast_variant_copyable_less_content)
	{
		fast_variant2<int, float> v(1), w(2);
		BOOST_CHECK(v < w);
	}

	// std::hash

	BOOST_AUTO_TEST_CASE(fast_variant_hash)
	{
		using variant = fast_variant2<int, float>;
		std::unordered_set<variant> s;
		s.insert(2);
		BOOST_CHECK_EQUAL(1, s.count(2));
		s.insert(3);
		BOOST_CHECK_EQUAL(1, s.count(3));
		s.erase(2);
		BOOST_CHECK_EQUAL(0, s.count(2));
		BOOST_CHECK_EQUAL(1, s.count(3));
		s.insert(2.0f);
		BOOST_CHECK_EQUAL(0, s.count(2));
		BOOST_CHECK_EQUAL(1, s.count(2.0f));
	}
}
