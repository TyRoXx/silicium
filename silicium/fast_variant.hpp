#ifndef SILICIUM_FAST_VARIANT_HPP
#define SILICIUM_FAST_VARIANT_HPP

#include <new>
#include <type_traits>
#include <silicium/override.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/mpl/max_element.hpp>
#include <boost/mpl/sizeof.hpp>
#include <boost/mpl/find.hpp>
#include <boost/optional.hpp>
#include <boost/variant/static_visitor.hpp>
#include <tuple>
#include <cassert>
#include <limits>

namespace Si
{
	template <class ...T>
	struct max_sizeof
	{
		static std::size_t const value = boost::mpl::max_element<
			boost::mpl::transform_view<
				boost::mpl::vector<T...>,
				boost::mpl::sizeof_<boost::mpl::_1>
			>
		>::type::type::value;
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
		//TODO: save an indirection by using function pointers instead of virtual methods
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

		static typename std::decay<Visitor>::type::result_type
		visit_const(Visitor &visitor, void const *storage)
		{
			return std::forward<Visitor>(visitor)(*static_cast<Element *>(storage));
		}
	};

	template <class T, class U>
	struct not_same_as : boost::mpl::not_<std::is_same<T, U>>
	{
	};

	template <class ...T>
	struct fast_variant
	{
		fast_variant() BOOST_NOEXCEPT
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

		fast_variant(fast_variant &&other) BOOST_NOEXCEPT
			: type(other.type)
		{
			type->move_construct(&storage, &other.storage);
		}

		fast_variant &operator = (fast_variant &&other) BOOST_NOEXCEPT
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

		template <
			class U,
			class CleanU = typename std::decay<U>::type,
			size_t Index = index_of<CleanU, T...>::value,
			class NoFastVariant = typename std::enable_if<not_same_as<CleanU, fast_variant<T...>>::value, void>::type>
		fast_variant(U &&value) BOOST_NOEXCEPT_IF(std::is_rvalue_reference<U>::value || std::is_nothrow_copy_constructible<CleanU>::value)
			: type(&get_type<Index>())
		{
			construct_impl<Index>(std::is_rvalue_reference<U>(), value);
		}

		template <
			class U,
			class CleanU = typename std::decay<U>::type,
			size_t Index = index_of<CleanU, T...>::value,
			class NoFastVariant = typename std::enable_if<not_same_as<CleanU, fast_variant<T...>>::value, void>::type>
		fast_variant &operator = (U &&other) BOOST_NOEXCEPT_IF(std::is_rvalue_reference<U>::value || std::is_nothrow_copy_assignable<CleanU>::value)
		{
			assignment_impl<Index>(std::is_rvalue_reference<U>(), other);
			return *this;
		}

		~fast_variant() BOOST_NOEXCEPT
		{
			assert(type);
			type->destroy(&storage);
		}

		int which() const BOOST_NOEXCEPT
		{
			return type->which();
		}

		template <class Visitor>
		auto apply_visitor(Visitor &&visitor) -> typename std::decay<Visitor>::type::result_type
		{
			using result_type = typename std::decay<Visitor>::type::result_type;
			using visit_fn = result_type (*)(Visitor &, void *);
			visit_fn const f[] = {&visitor_caller<Visitor, T>::visit...};
			return f[which()](visitor, &storage);
		}

		template <class Visitor>
		auto apply_visitor(Visitor &&visitor) const -> typename std::decay<Visitor>::type::result_type
		{
			using result_type = typename std::decay<Visitor>::type::result_type;
			using visit_fn = result_type (*)(Visitor &, void const *);
			visit_fn const f[] = {&visitor_caller<Visitor, typename std::add_const<T>::type>::visit_const...};
			return f[which()](visitor, &storage);
		}

	private:

		fast_variant_vtable const *type;
		typename std::aligned_storage<max_sizeof<T...>::value>::type storage;

		template <std::size_t Index>
		static fast_variant_vtable const &get_type() BOOST_NOEXCEPT
		{
			typedef typename std::tuple_element<Index, std::tuple<T...>>::type element;
			static fast_variant_vtable_impl<element, Index> const instance;
			return instance;
		}

		template <std::size_t Index, class U>
		void construct_impl(std::true_type, U &value) BOOST_NOEXCEPT
		{
			type->move_construct(&storage, &value);
		}

		template <std::size_t Index, class U>
		void construct_impl(std::false_type, U const &value)
		{
			type->copy_construct(&storage, &value);
		}

		template <std::size_t Index, class U>
		void assignment_impl(std::true_type, U &value) BOOST_NOEXCEPT
		{
			type->destroy(&storage);
			type = &get_type<Index>();
			type->move_construct(&storage, &value);
		}

		template <std::size_t Index, class U>
		void assignment_impl(std::false_type, U const &value)
		{
			fast_variant copy(value);
			*this = std::move(copy);
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

	struct equal_to
	{
		template <class T>
		bool operator()(T const &left, T const &right) const
		{
			return (left == right);
		}
	};

	struct less
	{
		template <class T>
		bool operator()(T const &left, T const &right) const
		{
			return (left < right);
		}
	};

	template <class Comparison, class ...T>
	struct comparison_visitor : boost::static_visitor<bool>
	{
		fast_variant<T...> const *other = nullptr;

		explicit comparison_visitor(fast_variant<T...> const &other) BOOST_NOEXCEPT
			: other(&other)
		{
		}

		template <class Element>
		bool operator()(Element &value) const BOOST_NOEXCEPT
		{
			auto other_value = try_get_ptr<Element>(*other);
			assert(other_value);
			return Comparison()(value, *other_value);
		}
	};

	template <class ...T>
	bool operator == (fast_variant<T...> const &left, fast_variant<T...> const &right) BOOST_NOEXCEPT
	{
		if (left.which() != right.which())
		{
			return false;
		}
		comparison_visitor<equal_to, T...> v{right};
		return Si::apply_visitor(v, left);
	}

	template <class ...T>
	bool operator < (fast_variant<T...> const &left, fast_variant<T...> const &right) BOOST_NOEXCEPT
	{
		if (left.which() != right.which())
		{
			return left.which() < right.which();
		}
		comparison_visitor<less, T...> v{right};
		return Si::apply_visitor(v, left);
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

#endif
