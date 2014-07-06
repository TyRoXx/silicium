#ifndef SILICIUM_FAST_VARIANT_HPP
#define SILICIUM_FAST_VARIANT_HPP

#include <new>
#include <type_traits>
#include <silicium/override.hpp>
#include <boost/mpl/min_max.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/find.hpp>
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
}

#endif
