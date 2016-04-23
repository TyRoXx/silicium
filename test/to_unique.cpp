#include <silicium/to_unique.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(to_unique)
{
    std::unique_ptr<long> p = Si::to_unique(2L);
    BOOST_REQUIRE(p);
    BOOST_CHECK_EQUAL(2, *p);
}

struct base
{
    virtual ~base()
    {
    }
};

struct derived : base
{
};

BOOST_AUTO_TEST_CASE(to_unique_explicit_pointee)
{
    std::unique_ptr<derived> p = Si::to_unique(derived());
    BOOST_REQUIRE(p);

    std::unique_ptr<base> q = Si::to_unique<base>(derived());
    BOOST_REQUIRE(q);
}
