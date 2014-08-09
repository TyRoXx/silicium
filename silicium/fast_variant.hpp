#ifndef SILICIUM_FAST_VARIANT_HPP
#define SILICIUM_FAST_VARIANT_HPP

#include <new>
#include <array>
#include <memory>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/optional.hpp>

namespace Si
{
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
			using which_type = unsigned;

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
						boost::mpl::bool_<(index_of<CleanU, T...>::value < sizeof...(T))>
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

			using storage_type = std::array<char, sizeof(union_<T...>)>; //TODO: ensure proper alignment

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
						boost::mpl::bool_<(index_of<CleanU, T...>::value < sizeof...(T))>
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
	struct fast_variant : detail::select_fast_variant_base<T...>
	{
		BOOST_STATIC_ASSERT_MSG(detail::are_noexcept_movable<T...>::value, "All contained types must be nothrow/noexcept-movable");

		using base = detail::select_fast_variant_base<T...>;

		using base::base;

		fast_variant()
		{
		}
	};

	template <class ...T>
	bool operator != (fast_variant<T...> const &left, fast_variant<T...> const &right)
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

	template <class Visitor, class Variant>
	auto apply_visitor(Visitor &&visitor, Variant &&variant) -> typename std::decay<Visitor>::type::result_type
	{
		return std::forward<Variant>(variant).apply_visitor(std::forward<Visitor>(visitor));
	}

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
		return apply_visitor(try_get_ptr_visitor<Element>{}, from);
	}

	template <class Element, class ...T>
	Element *try_get_ptr(fast_variant<T...> const &from) BOOST_NOEXCEPT
	{
		return apply_visitor(try_get_ptr_visitor<typename std::add_const<Element>::type>{}, from);
	}

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

	template <class ...T>
	std::ostream &operator << (std::ostream &out, fast_variant<T...> const &v)
	{
		Si::apply_visitor(ostream_visitor{out}, v);
		return out;
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

BOOST_STATIC_ASSERT(sizeof(Si::fast_variant<int>) == 8);
BOOST_STATIC_ASSERT(sizeof(Si::fast_variant<int *>) == (4 + sizeof(int *)));

#endif
