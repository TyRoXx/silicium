#ifndef SILICIUM_DELEGATOR_GENERATE_HPP
#define SILICIUM_DELEGATOR_GENERATE_HPP

// enclosing namespace struct:

#define DELEGATOR_TEMPLATE(...) template <__VA_ARGS__>
#define DELEGATOR_NAME(name)                                                                                           \
	struct name                                                                                                        \
	{
#define DELEGATOR_TYPEDEF(...) __VA_ARGS__
#define DELEGATOR_BASIC_METHOD(name, constness, result, ...)

#include DELEGATOR_INCLUDE

#undef DELEGATOR_TEMPLATE
#undef DELEGATOR_NAME
#undef DELEGATOR_TYPEDEF
#undef DELEGATOR_BASIC_METHOD

// abstract base class:

#define DELEGATOR_TEMPLATE(...)
#define DELEGATOR_NAME(name)                                                                                           \
	struct abstract_base                                                                                               \
	{                                                                                                                  \
		virtual ~abstract_base()                                                                                       \
		{                                                                                                              \
		}
#define DELEGATOR_TYPEDEF(...) __VA_ARGS__
#define DELEGATOR_BASIC_METHOD(name, constness, result, ...) virtual auto name(__VA_ARGS__) constness->result = 0;

#include DELEGATOR_INCLUDE
}
;

#undef DELEGATOR_TEMPLATE
#undef DELEGATOR_NAME
#undef DELEGATOR_TYPEDEF
#undef DELEGATOR_BASIC_METHOD

// eraser:

#define DELEGATOR_TEMPLATE(...)
#define DELEGATOR_NAME(name)                                                                                           \
	template <class Impl>                                                                                              \
	struct eraser : abstract_base                                                                                      \
	{                                                                                                                  \
		explicit eraser(Impl impl)                                                                                     \
		    : m_impl(std::move(impl))                                                                                  \
		{                                                                                                              \
		}                                                                                                              \
                                                                                                                       \
	private:                                                                                                           \
		Impl m_impl;                                                                                                   \
                                                                                                                       \
	public:
#define DELEGATOR_TYPEDEF(...)
#define DELEGATOR_BASIC_METHOD_DETAIL_PARAMETER(r, data, i, elem) elem BOOST_PP_CAT(arg, i)
#define DELEGATOR_BASIC_METHOD_DETAIL_FORWARD(r, data, i, elem) std::forward<elem>(BOOST_PP_CAT(arg, i))
#define DELEGATOR_BASIC_METHOD(name, constness, result, ...)                                                           \
	virtual auto name(                                                                                                 \
	    BOOST_PP_LIST_FOR_EACH_I(DELEGATOR_BASIC_METHOD_DETAIL_PARAMETER, _, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__)))  \
	    constness->result SILICIUM_OVERRIDE                                                                            \
	{                                                                                                                  \
		return m_impl.name(BOOST_PP_LIST_FOR_EACH_I(DELEGATOR_BASIC_METHOD_DETAIL_FORWARD, _,                          \
		                                            BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__)));                          \
	}

#include DELEGATOR_INCLUDE
}
;

template <class Impl>
static eraser<typename std::decay<Impl>::type> erase(Impl &&impl)
{
	return eraser<typename std::decay<Impl>::type>(std::forward<Impl>(impl));
}

#undef DELEGATOR_TEMPLATE
#undef DELEGATOR_NAME
#undef DELEGATOR_TYPEDEF
#undef DELEGATOR_BASIC_METHOD_DETAIL_PARAMETER
#undef DELEGATOR_BASIC_METHOD_DETAIL_FORWARD
#undef DELEGATOR_BASIC_METHOD

struct fat_ref
{
	template <class Impl>
	explicit fat_ref(Impl &impl)
	    : m_impl(&impl)
	{
		// initialize the vtable
		static vtable const instance = {
#define DELEGATOR_TEMPLATE(...)
#define DELEGATOR_NAME(name)
#define DELEGATOR_TYPEDEF(...)
#define DELEGATOR_BASIC_METHOD_DETAIL_PARAMETER(r, data, i, elem) elem BOOST_PP_CAT(arg, i)
#define DELEGATOR_BASIC_METHOD_DETAIL_FORWARD(r, data, i, elem) std::forward<elem>(BOOST_PP_CAT(arg, i))
#define DELEGATOR_BASIC_METHOD(name, constness, result, ...)                                                           \
	[](void constness *impl, BOOST_PP_LIST_FOR_EACH_I(DELEGATOR_BASIC_METHOD_DETAIL_PARAMETER, _,                      \
	                                                  BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))) -> result               \
	{                                                                                                                  \
		return static_cast<Impl constness *>(impl)->name(BOOST_PP_LIST_FOR_EACH_I(                                     \
		    DELEGATOR_BASIC_METHOD_DETAIL_FORWARD, _, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__)));                        \
	},

#include DELEGATOR_INCLUDE

#undef DELEGATOR_TEMPLATE
#undef DELEGATOR_NAME
#undef DELEGATOR_TYPEDEF
#undef DELEGATOR_BASIC_METHOD_DETAIL_PARAMETER
#undef DELEGATOR_BASIC_METHOD_DETAIL_FORWARD
#undef DELEGATOR_BASIC_METHOD
		};
		m_virtuals = &instance;
	}

// methods:

#define DELEGATOR_TEMPLATE(...)
#define DELEGATOR_NAME(name)
#define DELEGATOR_TYPEDEF(...) __VA_ARGS__
#define DELEGATOR_BASIC_METHOD_DETAIL_PARAMETER(r, data, i, elem) elem BOOST_PP_CAT(arg, i)
#define DELEGATOR_BASIC_METHOD_DETAIL_FORWARD(r, data, i, elem) std::forward<elem>(BOOST_PP_CAT(arg, i))
#define DELEGATOR_BASIC_METHOD(name, constness, result, ...)                                                           \
	auto name(BOOST_PP_LIST_FOR_EACH_I(DELEGATOR_BASIC_METHOD_DETAIL_PARAMETER, _,                                     \
	                                   BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))) constness->result                      \
	{                                                                                                                  \
		return m_virtuals->name(m_impl, BOOST_PP_LIST_FOR_EACH_I(DELEGATOR_BASIC_METHOD_DETAIL_FORWARD, _,             \
		                                                         BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__)));             \
	}

#include DELEGATOR_INCLUDE

#undef DELEGATOR_TEMPLATE
#undef DELEGATOR_NAME
#undef DELEGATOR_TYPEDEF
#undef DELEGATOR_BASIC_METHOD_DETAIL_PARAMETER
#undef DELEGATOR_BASIC_METHOD_DETAIL_FORWARD
#undef DELEGATOR_BASIC_METHOD

private:
	void *m_impl;

	struct vtable
	{
#define DELEGATOR_TEMPLATE(...)
#define DELEGATOR_NAME(name)
#define DELEGATOR_TYPEDEF(...)
#define DELEGATOR_BASIC_METHOD(name, constness, result, ...) result (*name)(void constness *, __VA_ARGS__);

#include DELEGATOR_INCLUDE

#undef DELEGATOR_TEMPLATE
#undef DELEGATOR_NAME
#undef DELEGATOR_TYPEDEF
#undef DELEGATOR_BASIC_METHOD
	};
	vtable const *m_virtuals;
};

// end of enclosing namespace struct:
}
;

#endif
