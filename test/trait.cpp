#include <silicium/trait.hpp>
#include <boost/test/unit_test.hpp>

typedef long element;

SILICIUM_TRAIT(
	Producer,
	((get, (0, ()), element))
)

struct test_producer
{
	element get()
	{
		return 42;
	}
};

BOOST_AUTO_TEST_CASE(trivial_trait)
{
	std::unique_ptr<Producer::interface> p = Si::to_unique(Producer::erase(test_producer{}));
	BOOST_REQUIRE(p);
	BOOST_CHECK_EQUAL(42, p->get());
}

template <class T>
SILICIUM_TRAIT(
	Container,
	((emplace_back, (1, (T)), void))
	((resize, (1, (size_t)), void))
	((resize, (2, (size_t, T const &)), void))
	((empty, (0, ()), bool, const))
	((size, (0, ()), size_t, const BOOST_NOEXCEPT))
)

BOOST_AUTO_TEST_CASE(templatized_trait)
{
	auto container = Container<int>::erase(std::vector<int>{});
	container.emplace_back(123);
	{
		std::vector<int> const expected{123};
		BOOST_CHECK(expected == container.original);
	}
	container.resize(2);
	{
		std::vector<int> const expected{123, 0};
		BOOST_CHECK(expected == container.original);
	}
	container.resize(3, 7);
	{
		std::vector<int> const expected{123, 0, 7};
		BOOST_CHECK(expected == container.original);
	}
}

BOOST_AUTO_TEST_CASE(trait_const_method)
{
	auto container = Container<int>::erase(std::vector<int>{});
	auto const &const_ref = container;
	BOOST_CHECK(const_ref.empty());
	container.original.resize(1);
	BOOST_CHECK(!const_ref.empty());
}

#if SILICIUM_COMPILER_HAS_WORKING_NOEXCEPT
BOOST_AUTO_TEST_CASE(trait_noexcept_method)
{
	auto container = Container<int>::erase(std::vector<int>{});
	auto const &const_ref = container;
	BOOST_CHECK_EQUAL(0, const_ref.size());
	container.original.resize(3);
	BOOST_CHECK_EQUAL(3, const_ref.size());
	BOOST_STATIC_ASSERT(BOOST_NOEXCEPT_EXPR(const_ref.size()));
}
#endif

BOOST_AUTO_TEST_CASE(trait_box)
{
	//default constructor is available:
	Container<int>::box container;

	{
		//move construction is available:
		Container<int>::box container2 = Container<int>::make_box(std::vector<int>{});

		//move assignment is available:
		container = std::move(container2);

		BOOST_REQUIRE(container.original);
		BOOST_CHECK(!container2.original);
	}

	container.emplace_back(3);
	BOOST_CHECK(!container.empty());
	BOOST_CHECK_EQUAL(1u, container.size());
}

BOOST_AUTO_TEST_CASE(trait_eraser)
{
	//default constructor is available:
	Container<int>::eraser<std::vector<int>> container;

	{
		//move construction is available:
		Container<int>::eraser<std::vector<int>> container2 = Container<int>::erase(std::vector<int>{1, 2, 3});

		BOOST_CHECK_EQUAL(0u, container.original.size());
		BOOST_CHECK_EQUAL(3u, container2.original.size());

		//move assignment is available:
		container = std::move(container2);

		BOOST_CHECK_EQUAL(3u, container.original.size());
		BOOST_CHECK_EQUAL(0u, container2.original.size());
	}

	container.emplace_back(4);
	BOOST_CHECK(!container.empty());
	BOOST_CHECK_EQUAL(4u, container.size());
}

template <class Signature>
struct Callable;

template <class Result, class A0>
SILICIUM_SPECIALIZED_TRAIT(
	Callable,
	<Result(A0)>,
	,
	((operator(), (1, (A0)), Result))
)

BOOST_AUTO_TEST_CASE(trait_specialization)
{
	auto add_two = Callable<int(int)>::make_box([](int a) { return a + 2; });
	BOOST_CHECK_EQUAL(3, add_two(1));
}

SILICIUM_TRAIT_WITH_TYPEDEFS(
	WithTypedefs,
	typedef float element_type;
	,
	((get, (0, ()), element_type))
)

struct impl_with_typedefs
{
	float get()
	{
		return 12.0f;
	}
};

BOOST_AUTO_TEST_CASE(trait_with_typedefs)
{
	WithTypedefs::box b = WithTypedefs::make_box(impl_with_typedefs{});
	BOOST_STATIC_ASSERT((std::is_same<float, WithTypedefs::eraser<impl_with_typedefs>::element_type>::value));
	BOOST_STATIC_ASSERT((std::is_same<float, WithTypedefs::interface::element_type>::value));
	BOOST_STATIC_ASSERT((std::is_same<float, WithTypedefs::box::element_type>::value));
	BOOST_CHECK_EQUAL(12.0f, b.get());
}
