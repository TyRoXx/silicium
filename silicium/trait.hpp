#ifndef SILICIUM_TRAIT_HPP
#define SILICIUM_TRAIT_HPP

#include <silicium/to_unique.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#if SILICIUM_COMPILER_GENERATES_MOVES
#	define SILICIUM_MOVABLE_MEMBER(struct_name, member_name) \
	struct_name() = default; \
	SILICIUM_DEFAULT_MOVE(struct_name)
#else
#	define SILICIUM_MOVABLE_MEMBER(struct_name, member_name) \
	struct_name() : member_name() BOOST_NOEXCEPT {} \
	struct_name(struct_name &&other) BOOST_NOEXCEPT : member_name(std::move(other.member_name)) {} \
	struct_name &operator = (struct_name &&other) BOOST_NOEXCEPT { member_name = std::move(other.member_name); return *this; }
#endif

#define SILICIUM_DETAIL_MAKE_PARAMETER(z, n, array) BOOST_PP_COMMA_IF(n) BOOST_PP_ARRAY_ELEM(n, array) BOOST_PP_CAT(arg, n)
#define SILICIUM_DETAIL_MAKE_PARAMETERS(array) ( BOOST_PP_REPEAT(BOOST_PP_ARRAY_SIZE(array), SILICIUM_DETAIL_MAKE_PARAMETER, array) )

#define SILICIUM_DETAIL_MAKE_PURE_VIRTUAL_METHOD(r, data, elem) \
	virtual \
	BOOST_PP_TUPLE_ELEM(4, 2, elem) \
	BOOST_PP_TUPLE_ELEM(4, 0, elem) \
	SILICIUM_DETAIL_MAKE_PARAMETERS(BOOST_PP_TUPLE_ELEM(4, 1, elem)) \
	BOOST_PP_TUPLE_ELEM(4, 3, elem) \
	= 0;

#define SILICIUM_DETAIL_MAKE_INTERFACE(name, methods) struct name { \
	virtual ~name() {} \
	BOOST_PP_SEQ_FOR_EACH(SILICIUM_DETAIL_MAKE_PURE_VIRTUAL_METHOD, _, methods) \
};

#define SILICIUM_DETAIL_ERASER_METHOD_ARGUMENT(z, n, text) , BOOST_PP_CAT(_, n)

#define SILICIUM_DETAIL_MAKE_ERASER_METHOD(r, data, elem) \
	virtual \
	BOOST_PP_TUPLE_ELEM(4, 2, elem) \
	BOOST_PP_TUPLE_ELEM(4, 0, elem) \
	SILICIUM_DETAIL_MAKE_PARAMETERS(BOOST_PP_TUPLE_ELEM(4, 1, elem)) \
	BOOST_PP_TUPLE_ELEM(4, 3, elem) \
	SILICIUM_OVERRIDE { \
		return original. BOOST_PP_TUPLE_ELEM(4, 0, elem) ( \
			BOOST_PP_ENUM_PARAMS(BOOST_PP_ARRAY_SIZE(BOOST_PP_TUPLE_ELEM(4, 1, elem)), arg) \
		); \
		}

#define SILICIUM_DETAIL_MAKE_BOX_METHOD(r, data, elem) \
	BOOST_PP_TUPLE_ELEM(4, 2, elem) \
	BOOST_PP_TUPLE_ELEM(4, 0, elem) \
	SILICIUM_DETAIL_MAKE_PARAMETERS(BOOST_PP_TUPLE_ELEM(4, 1, elem)) \
	BOOST_PP_TUPLE_ELEM(4, 3, elem) { \
		assert(original); \
		return original -> BOOST_PP_TUPLE_ELEM(4, 0, elem) ( \
			BOOST_PP_ENUM_PARAMS(BOOST_PP_ARRAY_SIZE(BOOST_PP_TUPLE_ELEM(4, 1, elem)), arg) \
		); \
		}

#define SILICIUM_DETAIL_MAKE_ERASER(name, methods) template <class Original> struct name : interface { \
	Original original; \
	SILICIUM_MOVABLE_MEMBER(name, original) \
	explicit name(Original original) : original(std::move(original)) {} \
	BOOST_PP_SEQ_FOR_EACH(SILICIUM_DETAIL_MAKE_ERASER_METHOD, _, methods) \
};

#define SILICIUM_DETAIL_MAKE_BOX(name, methods) struct box { \
	std::unique_ptr<interface> original; \
	SILICIUM_MOVABLE_MEMBER(name, original) \
	explicit name(std::unique_ptr<interface> original) BOOST_NOEXCEPT : original(std::move(original)) {} \
	BOOST_PP_SEQ_FOR_EACH(SILICIUM_DETAIL_MAKE_BOX_METHOD, _, methods) \
};

#define SILICIUM_SPECIALIZED_TRAIT(name, specialization, methods) struct name specialization { \
	SILICIUM_DETAIL_MAKE_INTERFACE(interface, methods) \
	SILICIUM_DETAIL_MAKE_ERASER(eraser, methods) \
	SILICIUM_DETAIL_MAKE_BOX(box, methods) \
	template <class Original> \
	static eraser<typename std::decay<Original>::type> erase(Original &&original) { \
		return eraser<typename std::decay<Original>::type>{std::forward<Original>(original)}; \
		} \
	template <class Original> \
	static box make_box(Original &&original) { \
		return box{Si::to_unique(erase(std::forward<Original>(original)))}; \
		} \
};

#define SILICIUM_TRAIT(name, methods) SILICIUM_SPECIALIZED_TRAIT(name, , methods)

#endif
