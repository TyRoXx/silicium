#ifndef SILICIUM_FAST_VARIANT_HPP
#define SILICIUM_FAST_VARIANT_HPP

#include <new>
#include <type_traits>
#include <silicium/override.hpp>
#include <boost/mpl/min_max.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/find.hpp>
#include <boost/optional.hpp>
#include <boost/variant/static_visitor.hpp>
#include <tuple>
#include <cassert>
#include <limits>

namespace Si
{
	template <class ...T>
	struct max_sizeof;

	template <class Head, class ...Tail>
	struct max_sizeof<Head, Tail...> : boost::mpl::max<boost::mpl::int_<sizeof(Head)>, typename max_sizeof<Tail...>::type>::type
	{
	};

	template <>
	struct max_sizeof<> : boost::mpl::int_<0>
	{
	};

	template <class First, class ...T>
	struct first
	{
		typedef First type;
	};

	template <class Element, class ...All>
	struct index_of : boost::mpl::find<boost::mpl::vector<All...>, Element>::type::pos
	{
	};

	struct fast_variant_vtable
	{
		virtual ~fast_variant_vtable()
		{
		}
		virtual int which() const = 0;
		virtual void default_construct(void *storage) const = 0;
		virtual void destroy(void *storage) const = 0;
		virtual void copy(void *to, void const *from) const = 0;
		virtual void copy_construct(void *to, void const *from) const = 0;
		virtual void move_construct(void *to, void *from) const = 0;
	};

	template <class Element, std::size_t Index>
	struct fast_variant_vtable_impl : fast_variant_vtable
	{
		static_assert(std::is_nothrow_move_assignable<Element>::value, "noexcept move assignment operator required");
		static_assert(std::is_nothrow_move_constructible<Element>::value, "noexcept move constructor required");

		virtual int which() const SILICIUM_OVERRIDE
		{
			return Index;
		}

		virtual void default_construct(void *storage) const SILICIUM_OVERRIDE
		{
			new (static_cast<Element *>(storage)) Element();
		}

		virtual void destroy(void *storage) const SILICIUM_OVERRIDE
		{
			static_cast<Element *>(storage)->~Element();
		}

		virtual void copy(void *to, void const *from) const SILICIUM_OVERRIDE
		{
			*static_cast<Element *>(to) = *static_cast<Element const *>(from);
		}

		virtual void copy_construct(void *to, void const *from) const SILICIUM_OVERRIDE
		{
			new (static_cast<Element *>(to)) Element(*static_cast<Element const *>(from));
		}

		virtual void move_construct(void *to, void *from) const SILICIUM_OVERRIDE
		{
			new (static_cast<Element *>(to)) Element(std::move(*static_cast<Element *>(from)));
		}
	};

	template <class Visitor, class Element>
	struct visitor_caller
	{
		static typename std::decay<Visitor>::type::result_type
		visit(Visitor &visitor, void *storage)
		{
			return std::forward<Visitor>(visitor)(*static_cast<Element *>(storage));
		}
	};

	template <class ...T>
	struct fast_variant
	{
		fast_variant()
			: type(&get_type<0>())
		{
			typedef typename std::tuple_element<0, std::tuple<T...>>::type first_element;
			static_assert(std::is_nothrow_default_constructible<first_element>::value, "noexcept default constructor required");
			type->default_construct(&storage);
		}

		fast_variant(fast_variant const &other)
			: type(other.type)
		{
			type->copy_construct(&storage, &other.storage);
		}

		fast_variant &operator = (fast_variant const &other)
		{
			if (this == &other)
			{
				return *this;
			}
			fast_variant copy(other);
			*this = std::move(copy);
			return *this;
		}

		fast_variant(fast_variant &&other)
			: type(other.type)
		{
			type->move_construct(&storage, &other.storage);
		}

		fast_variant &operator = (fast_variant &&other)
		{
			if (this == &other)
			{
				return *this;
			}
			type->destroy(&storage);
			type = other.type;
			type->move_construct(&storage, &other.storage);
			return *this;
		}

		template <class U, size_t Index = index_of<typename std::decay<U>::type, T...>::value>
		fast_variant(U &&value)
			: type(&get_type<Index>())
		{
			type->move_construct(&storage, &value);
		}

		~fast_variant()
		{
			assert(type);
			type->destroy(&storage);
		}

		int which() const
		{
			return type->which();
		}

		template <class Visitor>
		auto apply_visitor(Visitor &&visitor)
		{
			using result_type = typename std::decay<Visitor>::type::result_type;
			using visit_fn = result_type (*)(Visitor &, void *);
			visit_fn const f[] = {&visitor_caller<Visitor, T>::visit...};
			return f[which()](visitor, &storage);
		}

	private:

		fast_variant_vtable const *type;
		typename std::aligned_storage<max_sizeof<T...>::value>::type storage;

		template <std::size_t Index>
		fast_variant_vtable const &get_type()
		{
			typedef typename std::tuple_element<Index, std::tuple<T...>>::type element;
			static fast_variant_vtable_impl<element, Index> const instance;
			return instance;
		}
	};

	template <class Visitor, class Variant>
	auto apply_visitor(Visitor &&visitor, Variant &&variant)
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
}

#endif
