#include <boost/preprocessor.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <silicium/to_unique.hpp>

namespace Si
{

}

#define SILICIUM_DETAIL_REMOVE_PARENTHESES(...) __VA_ARGS__

#define SILICIUM_DETAIL_MAKE_PURE_VIRTUAL_METHOD(r, data, elem) \
	virtual \
	BOOST_PP_TUPLE_ELEM(3, 2, elem) \
	BOOST_PP_TUPLE_ELEM(3, 0, elem) \
	BOOST_PP_TUPLE_ELEM(3, 1, elem) \
	= 0;

#define SILICIUM_DETAIL_MAKE_INTERFACE(name, methods) struct name { \
	virtual ~name() {} \
	BOOST_PP_SEQ_FOR_EACH(SILICIUM_DETAIL_MAKE_PURE_VIRTUAL_METHOD, _, methods) \
};

#define SILICIUM_DETAIL_MAKE_ERASER_METHOD(r, data, elem) \
	virtual \
	BOOST_PP_TUPLE_ELEM(3, 2, elem) \
	BOOST_PP_TUPLE_ELEM(3, 0, elem) \
	BOOST_PP_TUPLE_ELEM(3, 1, elem) \
	SILICIUM_OVERRIDE { \
		return original. BOOST_PP_TUPLE_ELEM(3, 0, elem) (); \
	}

#define SILICIUM_DETAIL_MAKE_ERASER(name, methods) template <class Original> struct name : interface { \
	Original original; \
	explicit name(Original original) : original(std::move(original)) {} \
	BOOST_PP_SEQ_FOR_EACH(SILICIUM_DETAIL_MAKE_ERASER_METHOD, _, methods) \
};

#define SILICIUM_TRAIT(name, methods) struct name { \
	SILICIUM_DETAIL_MAKE_INTERFACE(interface, methods) \
	SILICIUM_DETAIL_MAKE_ERASER(eraser, methods) \
	template <class Original> \
	static auto erase(Original &&original) { \
		return eraser<typename std::decay<Original>::type>{std::forward<Original>(original)}; \
	} \
};

typedef long element;

SILICIUM_TRAIT(
	Producer,
	((get, (), element))
)

struct test_producer
{
	element get()
	{
		return 42;
	}
};

BOOST_AUTO_TEST_CASE(trait)
{
	std::unique_ptr<Producer::interface> p = Si::to_unique(Producer::erase(test_producer{}));
	BOOST_REQUIRE(p);
	BOOST_CHECK_EQUAL(42, p->get());
}
