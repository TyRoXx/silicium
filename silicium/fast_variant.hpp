#ifndef SILICIUM_FAST_VARIANT_HPP
#define SILICIUM_FAST_VARIANT_HPP

#include <silicium/is_handle.hpp>
#include <silicium/detail/argument_of.hpp>
#include <new>
#include <array>
#include <memory>
#include <boost/variant/static_visitor.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/integer.hpp>
#include <boost/optional.hpp>
#include <boost/type_traits.hpp>

namespace Si
{
	template <class Visitor, class Variant>
	auto apply_visitor(Visitor &&visitor, Variant &&variant) -> typename std::decay<Visitor>::type::result_type
	{
		return std::forward<Variant>(variant).apply_visitor(std::forward<Visitor>(visitor));
	}

	template <class T>
	struct inplace {};

	namespace detail
	{
		template <class ...T>
		struct union_;

		template <class First, class ...T>
		struct union_<First, T...>
		{
			union
			{
#ifndef _MSC_VER
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 408
				alignas(First)
#endif
#endif
					std::array<char, sizeof(First)> head;
				union_<T...> tail;
			}
			content;
		};

		template <>
		struct union_<>
		{
		};

		template <class Which, std::size_t Size>
		struct combined_storage
		{
			typedef Which which_type;

			combined_storage() BOOST_NOEXCEPT
			{
				which(0);
			}

			which_type which() const BOOST_NOEXCEPT
			{
				return *reinterpret_cast<which_type const *>(bytes.data() + which_offset);
			}

			void which(which_type value) BOOST_NOEXCEPT
			{
				*reinterpret_cast<which_type *>(bytes.data() + which_offset) = value;
			}

			char &storage() BOOST_NOEXCEPT
			{
				return *bytes.data();
			}

			char const &storage() const BOOST_NOEXCEPT
			{
				return *bytes.data();
			}

		private:

			typedef unsigned alignment_unit;
			enum
			{
				which_offset = (Size + sizeof(which_type) - 1) / sizeof(which_type) * sizeof(which_type),
				used_size = which_offset + sizeof(which_type),
				padding = sizeof(alignment_unit),
				padded_size = (used_size + padding - 1) / padding * padding
			};
			std::array<char, padded_size> bytes;
		};

		template <class T>
		void destroy_storage(void *storage) BOOST_NOEXCEPT
		{
#ifdef _MSC_VER
			//VC++ 2013: Silence (wrong) warning about unreferenced parameter
			(void)storage;
#endif
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

#if SILICIUM_COMPILER_HAS_VARIADIC_PACK_EXPANSION
		template <class First, class ...Rest>
		struct first
		{
			typedef First type;
		};
#else
		template <class ...Sequence>
		struct first;

		template <class First, class ...Rest>
		struct first<First, Rest...>
		{
			typedef First type;
		};
#endif

#if SILICIUM_COMPILER_HAS_VARIADIC_PACK_EXPANSION
		template <class ...All>
		struct make_mpl_vector
		{
			typedef boost::mpl::vector<All...> type;
		};
#else
		//workaround for GCC 4.6

		template <class ...All>
		struct make_mpl_vector;

		template <class Head, class ...Tail>
		struct make_mpl_vector<Head, Tail...>
		{
			typedef typename boost::mpl::push_front<
				typename make_mpl_vector<Tail...>::type,
				Head
			>::type type;
		};

		template <>
		struct make_mpl_vector<>
		{
			typedef boost::mpl::vector<> type;
		};
#endif

		template <class Element, class ...All>
		struct index_of : boost::mpl::find<typename make_mpl_vector<All...>::type, Element>::type::pos
		{
		};

		template <class ...T>
		struct are_noexcept_movable;

		template <class First, class ...T>
		struct are_noexcept_movable<First, T...>
			: boost::mpl::and_<
				boost::mpl::and_<
#if BOOST_VERSION >= 105400
					boost::is_nothrow_move_constructible<First>,
					boost::is_nothrow_move_assignable<First>
#else
					std::is_nothrow_move_constructible<First>,
					std::is_nothrow_move_assignable<First>
#endif
				>,
				are_noexcept_movable<T...>
			>::type
		{
		};

		template <>
		struct are_noexcept_movable<> : std::true_type
		{
		};
		
		template <class Constructed>
		struct construction_visitor : boost::static_visitor<Constructed>
		{
			template <class T>
			Constructed operator()(T &&argument) const
			{
				return Constructed(std::forward<T>(argument));
			}
		};

		template <bool IsCopyable, class ...T>
		struct fast_variant_base;

		template <class ...T>
		struct fast_variant_base<false, T...>
		{
			enum
			{
				is_noexcept_movable =
#if SILICIUM_COMPILER_HAS_WORKING_NOEXCEPT
					detail::are_noexcept_movable<T...>::value
#else
					1
#endif
			};

			typedef unsigned which_type;
			typedef boost::mpl::vector<T...> element_types;

			fast_variant_base() BOOST_NOEXCEPT
			{
				typedef typename first<T...>::type constructed;
				new (reinterpret_cast<constructed *>(&storage.storage())) constructed();
			}

			~fast_variant_base() BOOST_NOEXCEPT
			{
				destroy_storage(storage.which(), storage.storage());
			}

			template <class U = void, class V = typename boost::enable_if_c<is_noexcept_movable, U>::type>
			fast_variant_base(fast_variant_base &&other) BOOST_NOEXCEPT
			{
				storage.which(other.storage.which());
				move_construct_storage(storage.which(), storage.storage(), other.storage.storage());
			}

			template <class U = void, class V = typename boost::enable_if_c<is_noexcept_movable, U>::type>
			fast_variant_base &operator = (fast_variant_base &&other) BOOST_NOEXCEPT
			{
				if (storage.which() == other.which())
				{
					move_storage(storage.which(), storage.storage(), other.storage.storage());
				}
				else
				{
					destroy_storage(storage.which(), storage.storage());
					move_construct_storage(other.storage.which(), storage.storage(), other.storage.storage());
					storage.which(other.storage.which());
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
						boost::mpl::bool_<(index_of<CleanU, T...>::value < sizeof...(T))>
					>::value,
					void
				>::type>
			fast_variant_base(U &&value)
			{
				storage.which(Index);
				new (&storage.storage()) CleanU(std::forward<U>(value));
			}

			template <
				class Requested,
				class ...Args,
				std::size_t Index = index_of<Requested, T...>::value
			>
			explicit fast_variant_base(inplace<Requested>, Args &&...args)
			{
				storage.which(Index);
				new (&storage.storage()) Requested(std::forward<Args>(args)...);
			}

			which_type which() const BOOST_NOEXCEPT
			{
				return storage.which();
			}

			template <class Visitor>
			auto apply_visitor(Visitor &&visitor) -> typename std::decay<Visitor>::type::result_type
			{
				typedef typename std::decay<Visitor>::type::result_type result;
				static std::array<result (*)(Visitor &&, void *), sizeof...(T)> const f
				{{
					&apply_visitor_impl<Visitor, T, result>...
				}};
				return f[storage.which()](std::forward<Visitor>(visitor), &storage.storage());
			}

			template <class Visitor>
			auto apply_visitor(Visitor &&visitor) const -> typename std::decay<Visitor>::type::result_type
			{
				typedef typename std::decay<Visitor>::type::result_type result;
				static std::array<result(*)(Visitor &&, void const *), sizeof...(T)> const f
				{{
					&apply_visitor_const_impl<Visitor, T, result>...
				}};
				return f[storage.which()](std::forward<Visitor>(visitor), &storage.storage());
			}

			bool operator == (fast_variant_base const &other) const
			{
				if (storage.which() != other.storage.which())
				{
					return false;
				}
				static std::array<bool(*)(void const *, void const *), sizeof...(T)> const f =
				{{
					&equals<T>...
				}};
				return f[storage.which()](&storage.storage(), &other.storage.storage());
			}

			bool operator < (fast_variant_base const &other) const
			{
				if (storage.which() > other.storage.which())
				{
					return false;
				}
				if (storage.which() < other.storage.which())
				{
					return true;
				}
				static std::array<bool(*)(void const *, void const *), sizeof...(T)> const f =
				{{
					&less<T>...
				}};
				return f[storage.which()](&storage.storage(), &other.storage.storage());
			}

			template <bool IsOtherCopyable, class ...U>
			void assign(fast_variant_base<IsOtherCopyable, U...> &&other)
			{
				*this = Si::apply_visitor(construction_visitor<fast_variant_base>(), std::move(other));
			}

		protected: //TODO: make private somehow

			typedef typename boost::uint_value_t<sizeof...(T)>::least internal_which_type;
			typedef combined_storage<internal_which_type, sizeof(union_<T...>)> storage_type; //TODO: ensure proper alignment

			storage_type storage;

			static void move_construct_storage(which_type which, char &destination, char &source) BOOST_NOEXCEPT
			{
				static std::array<void(*)(void *, void *), sizeof...(T)> const f =
				{{
					&detail::move_construct_storage<T>...
				}};
				f[which](&destination, &source);
			}

			static void destroy_storage(which_type which, char &destroyed) BOOST_NOEXCEPT
			{
				static std::array<void(*)(void *), sizeof...(T)> const f =
				{{
					&detail::destroy_storage<T>...
				}};
				f[which](&destroyed);
			}

			static void move_storage(which_type which, char &destination, char &source) BOOST_NOEXCEPT
			{
				static std::array<void(*)(void *, void *), sizeof...(T)> const f =
				{{
					&detail::move_storage<T>...
				}};
				f[which](&destination, &source);
			}

			SILICIUM_DELETED_FUNCTION(fast_variant_base(fast_variant_base const &))
			SILICIUM_DELETED_FUNCTION(fast_variant_base &operator = (fast_variant_base const &))
		};

		template <class ...T>
		struct fast_variant_base<true, T...> : fast_variant_base<false, T...>
		{
			typedef fast_variant_base<false, T...> base;
			typedef typename base::which_type which_type;

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
						boost::mpl::bool_<(index_of<CleanU, T...>::value < sizeof...(T))>
					>::value,
					void
				>::type>
			fast_variant_base(U &&value)
				: base(std::forward<U>(value))
			{
			}

			template <class Requested, class ...Args>
			explicit fast_variant_base(inplace<Requested> i, Args &&...args)
				: base(i, std::forward<Args>(args)...)
			{
			}

			fast_variant_base(fast_variant_base &&other) BOOST_NOEXCEPT
				: base(std::move(other))
			{
			}

			fast_variant_base(fast_variant_base const &other)
				: base()
			{
				this->storage.which(other.storage.which());
				copy_construct_storage(this->storage.which(), this->storage.storage(), other.storage.storage());
			}

			fast_variant_base &operator = (fast_variant_base &&other) BOOST_NOEXCEPT
			{
				base::operator = (std::move(other));
				return *this;
			}

			fast_variant_base &operator = (fast_variant_base const &other)
			{
				if (this->storage.which() == other.storage.which())
				{
					copy_storage(this->storage.which(), this->storage.storage(), other.storage.storage());
				}
				else
				{
					typename base::storage_type temporary;
					copy_construct_storage(other.storage.which(), temporary.storage(), other.storage.storage());
					base::destroy_storage(this->storage.which(), this->storage.storage());
					base::move_construct_storage(other.storage.which(), this->storage.storage(), temporary.storage());
					this->storage.which(other.storage.which());
				}
				return *this;
			}

			using base::assign;

			template <bool IsOtherCopyable, class ...U>
			void assign(fast_variant_base<IsOtherCopyable, U...> const &other)
			{
				*this = Si::apply_visitor(construction_visitor<fast_variant_base>(), other);
			}

		private:

			static void copy_construct_storage(which_type which, char &destination, char const &source)
			{
				static std::array<void(*)(void *, void const *), sizeof...(T)> const f =
				{{
					&detail::copy_construct_storage<T>...
				}};
				f[which](&destination, &source);
			}

			static void copy_storage(which_type which, char &destination, char const &source)
			{
				static std::array<void(*)(void *, void const *), sizeof...(T)> const f =
				{{
					&detail::copy_storage<T>...
				}};
				f[which](&destination, &source);
			}
		};

		template <class ...T>
		struct are_copyable;

		template <class First, class ...T>
		struct are_copyable<First, T...>
			: boost::mpl::and_<
				boost::mpl::and_<
					Si::is_copy_constructible<First>,
					Si::is_copy_assignable<First>
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
		BOOST_STATIC_ASSERT((are_copyable<int, float>::value));

		//In VC++ 2013 Update 3 the type traits is_copy_constructible and is_copy_assignable still return true for unique_ptr,
		//so this assert would fail.
#if SILICIUM_HAS_PROPER_COPY_TRAITS
		//GCC 4.6 with Boost < 1.55 is also a problem
		BOOST_STATIC_ASSERT((!are_copyable<int, std::unique_ptr<int>>::value));
#endif

#if SILICIUM_COMPILER_HAS_USING
		template <class ...T>
		using select_fast_variant_base = fast_variant_base<are_copyable<T...>::value, T...>;
#else
		template <class ...T>
		struct select_fast_variant_base
		{
			typedef fast_variant_base<are_copyable<T...>::value, T...> type;
		};
#endif

		struct ostream_visitor : boost::static_visitor<>
		{
			std::ostream *out;

			explicit ostream_visitor(std::ostream &out)
				: out(&out)
			{
			}

			template <class T>
			void operator()(T const &value) const
			{
				*out << value;
			}
		};

		template <bool IsCopyable, class ...T>
		std::ostream &operator << (std::ostream &out, fast_variant_base<IsCopyable, T...> const &v)
		{
			Si::apply_visitor(ostream_visitor{ out }, v);
			return out;
		}
	}

#if SILICIUM_COMPILER_HAS_USING
	template <class ...T>
	using fast_variant = detail::select_fast_variant_base<T...>;
#else
	template <class ...T>
	struct fast_variant : detail::select_fast_variant_base<T...>::type
	{
		typedef typename detail::select_fast_variant_base<T...>::type base;

		fast_variant() BOOST_NOEXCEPT
		{
		}

		template <class First, class ...Initializer, class = typename boost::disable_if<boost::is_same<boost::decay<First>::type, fast_variant>, void>::type>
		fast_variant(First &&first, Initializer &&...init)
			: base(std::forward<First>(first), std::forward<Initializer>(init)...)
		{
		}

		fast_variant(fast_variant &&other) BOOST_NOEXCEPT
			: base(std::move(static_cast<base &>(other)))
		{
		}

		fast_variant(fast_variant const &other)
			: base(static_cast<base const &>(other))
		{
		}

		template <class Content, class = typename boost::disable_if<boost::is_same<boost::decay<Content>::type, fast_variant>, void>::type>
		fast_variant &operator = (Content &&other)
		{
			static_cast<base &>(*this) = std::forward<Content>(other);
			return *this;
		}

		fast_variant &operator = (fast_variant &&other) BOOST_NOEXCEPT
		{
			static_cast<base &>(*this) = std::move(static_cast<base &>(other));
			return *this;
		}

		fast_variant &operator = (fast_variant const &other)
		{
			static_cast<base &>(*this) = static_cast<base const &>(other);
			return *this;
		}

		template <class Other>
		void assign(Other &&other)
		{
			base::assign(static_cast<typename std::decay<Other>::type::base const &>(other));
		}
	};
#endif

	BOOST_STATIC_ASSERT(is_handle<fast_variant<int>>::value);
	BOOST_STATIC_ASSERT((is_handle<fast_variant<int, nothing>>::value));

	template <class ...T>
	bool operator != (fast_variant<T...> const &left, fast_variant<T...> const &right)
	{
		return !(left == right);
	}

	template <class ...T>
	bool operator > (fast_variant<T...> const &left, fast_variant<T...> const &right)
	{
		return (right < left);
	}

	template <class ...T>
	bool operator <= (fast_variant<T...> const &left, fast_variant<T...> const &right)
	{
		return !(left > right);
	}

	template <class ...T>
	bool operator >= (fast_variant<T...> const &left, fast_variant<T...> const &right)
	{
		return !(left < right);
	}

	struct hash_visitor : boost::static_visitor<std::size_t>
	{
		template <class T>
		std::size_t operator()(T const &element) const
		{
			return std::hash<T>()(element);
		}
	};

	template <class Element>
	struct try_get_visitor : boost::static_visitor<boost::optional<Element>>
	{
		boost::optional<Element> operator()(Element value) const
		{
			return std::move(value);
		}

		template <class Other>
		boost::optional<Element> operator()(Other const &) const
		{
			return boost::none;
		}
	};

	template <class Element, class ...T>
	boost::optional<Element> try_get(fast_variant<T...> &from)
	{
		return apply_visitor(try_get_visitor<Element>{}, from);
	}

	template <class Element>
	struct try_get_ptr_visitor : boost::static_visitor<Element *>
	{
		Element * operator()(Element &value) const BOOST_NOEXCEPT
		{
			return &value;
		}

		template <class Other>
		Element * operator()(Other const &) const BOOST_NOEXCEPT
		{
			return nullptr;
		}
	};

	template <class Element, class ...T>
	Element *try_get_ptr(fast_variant<T...> &from) BOOST_NOEXCEPT
	{
		BOOST_STATIC_ASSERT((boost::mpl::contains<boost::mpl::vector<T...>, Element>::value));
		return apply_visitor(try_get_ptr_visitor<Element>{}, from);
	}

	template <class Element, class ...T>
	typename std::add_const<Element>::type *try_get_ptr(fast_variant<T...> const &from) BOOST_NOEXCEPT
	{
		BOOST_STATIC_ASSERT((boost::mpl::contains<boost::mpl::vector<T...>, Element>::value));
		return apply_visitor(try_get_ptr_visitor<typename std::add_const<Element>::type>{}, from);
	}

	namespace detail
	{
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
	Result visit(fast_variant<T...> &variant, Visitors &&...visitors)
	{
		detail::overloader<Result, Visitors...> ov(visitors...);
		return Si::apply_visitor(ov, variant);
	}

	template <class Result, class ...T, class ...Visitors>
	Result visit(fast_variant<T...> const &variant, Visitors &&...visitors)
	{
		detail::overloader<Result, Visitors...> ov(visitors...);
		return Si::apply_visitor(ov, variant);
	}
}

namespace std
{
	template <class ...T>
	struct hash<Si::fast_variant<T...>>
	{
		std::size_t operator()(Si::fast_variant<T...> const &v) const
		{
			return Si::apply_visitor(Si::hash_visitor(), v);
		}
	};
}

//TODO: which is always 0, so this can be made as small as sizeof(int)
BOOST_STATIC_ASSERT(sizeof(Si::fast_variant<boost::uint32_t>) == (2 * sizeof(boost::uint32_t)));

//TODO: which is always 0, so this can be made as small as sizeof(int *)
BOOST_STATIC_ASSERT(sizeof(Si::fast_variant<int *>) == (sizeof(boost::uint32_t) + sizeof(int *)));

BOOST_STATIC_ASSERT(sizeof(Si::fast_variant<std::hash<Si::fast_variant<int>>>) == sizeof(boost::uint32_t));

#endif
