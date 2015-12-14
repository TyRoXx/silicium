#ifndef SILICIUM_DELEGATOR_GENERATE_HPP
#define SILICIUM_DELEGATOR_GENERATE_HPP

// enclosing namespace struct:

#define DELEGATOR_TEMPLATE(first, ...) template <first, __VA_ARGS__>
#define DELEGATOR_NAME(name)                                                                                           \
	struct name                                                                                                        \
	{
#define DELEGATOR_TYPEDEF(something, ...) something __VA_ARGS__
#define DELEGATOR_METHOD(name, result, ...)

#include DELEGATOR_INCLUDE

#undef DELEGATOR_TEMPLATE
#undef DELEGATOR_NAME
#undef DELEGATOR_TYPEDEF
#undef DELEGATOR_METHOD

// abstract base class:

#define DELEGATOR_TEMPLATE(first, ...)
#define DELEGATOR_NAME(name)                                                                                           \
	struct abstract_base                                                                                               \
	{                                                                                                                  \
		virtual ~abstract_base()                                                                                       \
		{                                                                                                              \
		}
#define DELEGATOR_TYPEDEF(something, ...) something __VA_ARGS__
#define DELEGATOR_METHOD(name, result, ...) virtual auto name(__VA_ARGS__)->result = 0;

#include DELEGATOR_INCLUDE
}
;

#undef DELEGATOR_TEMPLATE
#undef DELEGATOR_NAME
#undef DELEGATOR_TYPEDEF
#undef DELEGATOR_METHOD

// eraser:

#define DELEGATOR_TEMPLATE(first, ...)
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
#define DELEGATOR_TYPEDEF(something, ...)
#define DELEGATOR_METHOD_DETAIL_PARAMETER(r, data, i, elem) elem BOOST_PP_CAT(arg, i)
#define DELEGATOR_METHOD_DETAIL_FORWARD(r, data, i, elem) std::forward<elem>(BOOST_PP_CAT(arg, i))
#define DELEGATOR_METHOD(name, result, ...)                                                                            \
	virtual auto name(                                                                                                 \
	    BOOST_PP_LIST_FOR_EACH_I(DELEGATOR_METHOD_DETAIL_PARAMETER, _, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__)))        \
	    ->result override                                                                                              \
	{                                                                                                                  \
		return m_impl.name(                                                                                            \
		    BOOST_PP_LIST_FOR_EACH_I(DELEGATOR_METHOD_DETAIL_FORWARD, _, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__)));     \
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
#undef DELEGATOR_METHOD_DETAIL_PARAMETER
#undef DELEGATOR_METHOD_DETAIL_FORWARD
#undef DELEGATOR_METHOD

// end of enclosing namespace struct:
}
;

#endif
