#include <silicium/variant.hpp>
#include <silicium/make_unique.hpp>
#include <silicium/optional.hpp>
#include <silicium/noexcept_string.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/container/string.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <unordered_set>

#if SILICIUM_HAS_VARIANT
namespace Si
{
    BOOST_AUTO_TEST_CASE(variant_single)
    {
        variant<int> v;
        BOOST_CHECK_EQUAL(0U, v.index());
        int *p = try_get_ptr<int>(v);
        BOOST_REQUIRE(p);
        BOOST_CHECK_EQUAL(0, *p);
    }

    BOOST_AUTO_TEST_CASE(variant_assignment_same)
    {
        typedef variant<int, noexcept_string> variant;
        variant v(1), w(2);
        BOOST_CHECK_EQUAL(1, *try_get_ptr<int>(v));
        BOOST_CHECK_EQUAL(2, *try_get_ptr<int>(w));
        v = w;
        BOOST_CHECK_EQUAL(v.index(), w.index());
        BOOST_CHECK_EQUAL(v.index(), 0u);
        BOOST_CHECK_EQUAL(2, *try_get_ptr<int>(v));
        BOOST_CHECK_EQUAL(2, *try_get_ptr<int>(w));
    }

    BOOST_AUTO_TEST_CASE(variant_assignment_different)
    {
        variant<int, noexcept_string> v, w(noexcept_string("S"));
        BOOST_CHECK_EQUAL(0, *try_get_ptr<int>(v));
        BOOST_CHECK_EQUAL(
            noexcept_string("S"), *try_get_ptr<noexcept_string>(w));
        v = w;
        BOOST_CHECK_EQUAL(v.index(), w.index());
        BOOST_CHECK_EQUAL(v.index(), 1u);
        BOOST_CHECK_EQUAL(
            noexcept_string("S"), *try_get_ptr<noexcept_string>(v));
        BOOST_CHECK_EQUAL(
            noexcept_string("S"), *try_get_ptr<noexcept_string>(w));
    }

    struct is_int_visitor : boost::static_visitor<bool>
    {
        bool operator()(int) const
        {
            return true;
        }

        bool operator()(noexcept_string const &) const
        {
            return false;
        }
    };

    BOOST_AUTO_TEST_CASE(variant_apply_visitor)
    {
        {
            variant<int, noexcept_string> int_;
            is_int_visitor v;
            bool is_int = apply_visitor(v, int_);
            BOOST_CHECK(is_int);
        }

        {
            variant<int, noexcept_string> str(noexcept_string{});
            is_int_visitor v;
            bool is_int = apply_visitor(v, str);
            BOOST_CHECK(!is_int);
        }
    }

    BOOST_AUTO_TEST_CASE(variant_nesting)
    {
        variant<variant<int>, noexcept_string> f;
        f = noexcept_string("S");
        BOOST_CHECK_EQUAL(
            noexcept_string("S"), *try_get_ptr<noexcept_string>(f));

        f = variant<int>(2);
        BOOST_CHECK(variant<int>(2) == *try_get_ptr<variant<int>>(f));
    }

    BOOST_AUTO_TEST_CASE(variant_construct_const)
    {
        noexcept_string const s("S");
        variant<variant<int>, noexcept_string> f{s};
    }

    BOOST_AUTO_TEST_CASE(variant_assign_const)
    {
        variant<variant<int>, noexcept_string> f;
        noexcept_string const s("S");
        f = s;
    }

    BOOST_AUTO_TEST_CASE(variant_less)
    {
        variant<int, noexcept_string> f(1), g(2), h(noexcept_string("a")),
            i(noexcept_string("b"));
        BOOST_CHECK(f < g);
        BOOST_CHECK(f < h);
        BOOST_CHECK(f < i);
        BOOST_CHECK(h < i);
    }

    // default constructor

    BOOST_AUTO_TEST_CASE(variant_copyable_default)
    {
        typedef variant<int> variant;
        variant v;
        BOOST_CHECK_EQUAL(0U, v.index());
    }

    BOOST_AUTO_TEST_CASE(variant_non_copyable_default)
    {
        typedef variant<std::unique_ptr<int>> variant;
        variant v;
        BOOST_CHECK_EQUAL(0U, v.index());
    }

    // move constructor

    BOOST_AUTO_TEST_CASE(variant_non_copyable_construct_move)
    {
        typedef variant<std::unique_ptr<int>> variant;
        variant v(Si::make_unique<int>(2));
        variant w(std::move(v));
        BOOST_CHECK(v != w);
    }

    // copy constructor

    BOOST_AUTO_TEST_CASE(variant_copyable_construct_copy)
    {
        typedef variant<int> variant;
        variant v;
        variant w(v);
        BOOST_CHECK(v == w);
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_operator_copy)
    {
        typedef variant<int> variant;
        variant v;
        variant w(2);
        BOOST_CHECK(v != w);
        v = w;
        BOOST_CHECK(v == w);
    }

    // move operator

    BOOST_AUTO_TEST_CASE(variant_copyable_operator_move)
    {
        typedef variant<noexcept_string> variant;
        variant v;
        variant w(noexcept_string(1000, 'a'));
        BOOST_CHECK(v != w);
        v = std::move(w);
        BOOST_CHECK(v != w);
    }

    BOOST_AUTO_TEST_CASE(variant_non_copyable_operator_move)
    {
        typedef variant<std::unique_ptr<int>> variant;
        variant v;
        variant w(Si::make_unique<int>(2));
        BOOST_CHECK(v != w);
        v = std::move(w);
        BOOST_CHECK(v != w);
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_operator_move_to_self)
    {
        typedef variant<std::unique_ptr<int>, double> variant;
        variant v(Si::make_unique<int>(23));
        v = std::move(v);
        std::unique_ptr<int> *const content =
            Si::try_get_ptr<std::unique_ptr<int>>(v);
        BOOST_REQUIRE(content);
        BOOST_CHECK_EQUAL(23, **content);
    }

    // copy operator

    BOOST_AUTO_TEST_CASE(variant_copyable_operator_copy_to_same)
    {
        typedef variant<int, double> variant;
        variant v;
        variant w(3);
        BOOST_CHECK(v != w);
        v = w;
        BOOST_CHECK(v == w);
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_operator_copy_to_different)
    {
        typedef variant<int, double> variant;
        variant v(1.0);
        variant w(3);
        BOOST_CHECK(v != w);
        v = w;
        BOOST_CHECK(v == w);
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_operator_copy_raw)
    {
        typedef variant<int, double> variant;
        variant v;
        variant w(3);
        BOOST_CHECK(v != w);
        v = 3;
        BOOST_CHECK(v == w);
        w = 2.0;
        BOOST_CHECK(v != w);
        v = 2.0;
        BOOST_CHECK(v == w);
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_operator_copy_to_self)
    {
        typedef variant<std::shared_ptr<int>, double> variant;
        variant v(std::make_shared<int>(23));
        v = v;
        std::shared_ptr<int> *const content =
            Si::try_get_ptr<std::shared_ptr<int>>(v);
        BOOST_REQUIRE(content);
        BOOST_CHECK_EQUAL(23, **content);
    }

    // apply_visitor

    struct test_visitor_1 : boost::static_visitor<bool>
    {
        bool operator()(int i) const
        {
            return (i == 2);
        }

        bool operator()(double) const
        {
            return false;
        }
    };

    BOOST_AUTO_TEST_CASE(variant_copyable_apply_visitor_mutable)
    {
        typedef variant<int, double> variant;
        variant v(2);
        bool success = apply_visitor(test_visitor_1(), v);
        BOOST_CHECK(success);
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_apply_visitor_const)
    {
        typedef variant<int, double> variant;
        variant const v(2);
        bool success = apply_visitor(test_visitor_1(), v);
        BOOST_CHECK(success);
    }

    // comparison operators

    BOOST_AUTO_TEST_CASE(variant_copyable_equal)
    {
        variant<int, float> v, w;
        BOOST_CHECK(v == w);
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_not_equal)
    {
        variant<int, float> v, w, x(2), y(2.0f);
        BOOST_CHECK(!(v != w));
        BOOST_CHECK(v != x);
        BOOST_CHECK(x != y);
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_less_index)
    {
        variant<int, float> v(1), w(1.0f);
        BOOST_CHECK(v < w);
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_less_content)
    {
        variant<int, float> v(1), w(2);
        BOOST_CHECK(v < w);
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_less_equal_index)
    {
        variant<int, float> v(1), w(1.0f);
        BOOST_CHECK(v <= w);
        BOOST_CHECK(v <= v);
        BOOST_CHECK(w <= w);
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_less_equal_content)
    {
        variant<int, float> v(1), w(2);
        BOOST_CHECK(v <= w);
        BOOST_CHECK(v <= v);
        BOOST_CHECK(w <= w);
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_greater_index)
    {
        variant<int, float> v(1), w(1.0f);
        BOOST_CHECK(w > v);
        BOOST_CHECK(!(v > v));
        BOOST_CHECK(!(w > w));
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_greater_content)
    {
        variant<int, float> v(1), w(2);
        BOOST_CHECK(w > v);
        BOOST_CHECK(!(w > w));
        BOOST_CHECK(!(v > v));
        BOOST_CHECK(!(w > w));
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_greater_equal_index)
    {
        variant<int, float> v(1), w(1.0f);
        BOOST_CHECK(w >= v);
        BOOST_CHECK(v >= v);
        BOOST_CHECK(w >= w);
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_greater_equal_content)
    {
        variant<int, float> v(1), w(2);
        BOOST_CHECK(w >= v);
        BOOST_CHECK(v >= v);
        BOOST_CHECK(w >= w);
    }

    // std::hash

    BOOST_AUTO_TEST_CASE(variant_hash)
    {
        typedef variant<int, float> variant;
        std::unordered_set<variant> s;
        s.insert(2);
        BOOST_CHECK_EQUAL(1U, s.count(2));
        s.insert(3);
        BOOST_CHECK_EQUAL(1U, s.count(3));
        s.erase(2);
        BOOST_CHECK_EQUAL(0U, s.count(2));
        BOOST_CHECK_EQUAL(1U, s.count(3));
        s.insert(2.0f);
        BOOST_CHECK_EQUAL(0U, s.count(2));
        BOOST_CHECK_EQUAL(1U, s.count(2.0f));
    }

    // visit

    BOOST_AUTO_TEST_CASE(variant_const_visit)
    {
        typedef variant<int, float> variant;
        variant const v(2);
        BOOST_CHECK_EQUAL(2 + 1, visit<int>(v,
                                            [](int i) -> int
                                            {
                                                return i + 1;
                                            },
                                            [](float) -> int
                                            {
                                                BOOST_REQUIRE(false);
                                                return 0;
                                            }));
    }

    BOOST_AUTO_TEST_CASE(variant_mutable_visit)
    {
        typedef variant<int, float> variant;
        variant v(2);
        BOOST_CHECK_EQUAL(2 + 1, visit<int>(v,
                                            [](int i) -> int
                                            {
                                                return i + 1;
                                            },
                                            [](float) -> int
                                            {
                                                BOOST_REQUIRE(false);
                                                return 0;
                                            }));
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_mutable_assign_subset)
    {
        variant<int> v(2);
        variant<float, int> w;
        w.assign(v);
        BOOST_CHECK_EQUAL(2, *try_get_ptr<int>(w));
    }

    BOOST_AUTO_TEST_CASE(variant_copyable_const_assign_subset)
    {
        variant<int> const v(2);
        variant<float, int> w;
        w.assign(v);
        BOOST_CHECK_EQUAL(2, *try_get_ptr<int>(w));
    }

#if 0 // TODO: make this work
	BOOST_AUTO_TEST_CASE(variant_non_copyable_construct_superset)
	{
		variant<std::unique_ptr<int>> v(Si::make_unique<int>(2));
		variant<float, std::unique_ptr<int>> w;
		w.assign(std::move(v)); //would not compile because assign uses apply_visitor index does not forward rvalue-ness yet
		auto * const element = try_get_ptr<std::unique_ptr<int>>(w);
		BOOST_REQUIRE(element);
		BOOST_REQUIRE(*element);
		BOOST_CHECK_EQUAL(2, **element);
	}
#endif

    BOOST_AUTO_TEST_CASE(variant_overloaded_visit_unordered_with_const_visitors)
    {
        variant<int, unit, std::unique_ptr<long>> v(3);
        int result = visit<int>(v,
                                [](unit) -> int
                                {
                                    BOOST_FAIL("unexpected type");
                                    return 0;
                                },
                                [](int element) -> int
                                {
                                    return element;
                                },
                                [](std::unique_ptr<long> &) -> int
                                {
                                    BOOST_FAIL("unexpected type");
                                    return 0;
                                });
        BOOST_CHECK_EQUAL(3, result);
    }

    BOOST_AUTO_TEST_CASE(variant_visit_unordered_with_mutable_visitors)
    {
        variant<int, unit, std::unique_ptr<long>> v(3);
        int result = visit<int>(v,
                                [](unit) mutable -> int
                                {
                                    BOOST_FAIL("unexpected type");
                                    return 0;
                                },
                                [](int element) mutable -> int
                                {
                                    return element;
                                },
                                [](std::unique_ptr<long> &) mutable -> int
                                {
                                    BOOST_FAIL("unexpected type");
                                    return 0;
                                });
        BOOST_CHECK_EQUAL(3, result);
    }

    BOOST_AUTO_TEST_CASE(variant_visit_return_void)
    {
        variant<int, unit> v(3);
        bool got_result = false;
        visit<void>(v,
                    [](unit)
                    {
                        BOOST_FAIL("unexpected type");
                    },
                    [&got_result](int element)
                    {
                        BOOST_REQUIRE(!got_result);
                        got_result = true;
                        BOOST_CHECK_EQUAL(3, element);
                    });
        BOOST_CHECK(got_result);
    }

    struct needs_inplace_construction : boost::noncopyable
    {
        int v;

        explicit needs_inplace_construction(int v)
            : v(v)
        {
        }
    };

    bool operator==(needs_inplace_construction const &left,
                    needs_inplace_construction const &right)
    {
        return left.v == right.v;
    }

    std::ostream &operator<<(std::ostream &out,
                             needs_inplace_construction const &value)
    {
        return out << value.v;
    }

    std::ostream &operator<<(std::ostream &out, Si::unit)
    {
        return out << "unit";
    }

    BOOST_AUTO_TEST_CASE(variant_construct_inplace)
    {
        typedef variant<int, float, unit, std::string,
                        needs_inplace_construction> var;
        {
            var a(Si::inplace<int>(), 12);
            var b(12);
            BOOST_CHECK_EQUAL(b, a);
        }
        {
            var a(Si::inplace<float>(), 12.0f);
            var b(12.0f);
            BOOST_CHECK_EQUAL(b, a);
        }
        {
            var a((Si::inplace<unit>()), Si::unit());
            var b((Si::unit()));
            BOOST_CHECK_EQUAL(b, a);
        }
        {
            var a((Si::inplace<std::string>()), "test");
            var b(std::string("test"));
            BOOST_CHECK_EQUAL(b, a);
        }
        {
            var a((Si::inplace<needs_inplace_construction>()), 123);
            BOOST_CHECK_EQUAL(a, a);
            BOOST_CHECK_EQUAL(
                123, Si::visit<optional<int>>(
                         a,
                         [](int)
                         {
                             BOOST_FAIL("unexpected type");
                             return none;
                         },
                         [](float)
                         {
                             BOOST_FAIL("unexpected type");
                             return none;
                         },
                         [](unit)
                         {
                             BOOST_FAIL("unexpected type");
                             return none;
                         },
                         [](std::string const &)
                         {
                             BOOST_FAIL("unexpected type");
                             return none;
                         },
                         [](needs_inplace_construction const &value)
                         {
                             return value.v;
                         }));
        }
    }

    BOOST_AUTO_TEST_CASE(variant_std_ostream)
    {
        BOOST_CHECK_EQUAL(
            "123", boost::lexical_cast<std::string>(Si::variant<int>(123)));
    }
}

BOOST_AUTO_TEST_CASE(variant_try_get_ptr)
{
    Si::variant<Si::unit, int, std::unique_ptr<long>> u, v(123),
        w(Si::make_unique<long>(456));
    BOOST_STATIC_ASSERT(
        (std::is_same<Si::unit *,
                      decltype(Si::try_get_ptr<Si::unit>(u))>::value));
    BOOST_STATIC_ASSERT(
        (std::is_same<int *, decltype(Si::try_get_ptr<int>(u))>::value));
    BOOST_STATIC_ASSERT(
        (std::is_same<std::unique_ptr<long> *,
                      decltype(
                          Si::try_get_ptr<std::unique_ptr<long>>(u))>::value));
    BOOST_CHECK_NE(
        static_cast<Si::unit *>(nullptr), Si::try_get_ptr<Si::unit>(u));
    BOOST_CHECK_EQUAL(
        static_cast<Si::unit *>(nullptr), Si::try_get_ptr<Si::unit>(v));
    BOOST_CHECK_EQUAL(
        static_cast<Si::unit *>(nullptr), Si::try_get_ptr<Si::unit>(w));
    BOOST_CHECK_EQUAL(static_cast<int *>(nullptr), Si::try_get_ptr<int>(u));
    BOOST_REQUIRE_NE(static_cast<int *>(nullptr), Si::try_get_ptr<int>(v));
    BOOST_CHECK_EQUAL(static_cast<int *>(nullptr), Si::try_get_ptr<int>(w));
    BOOST_CHECK_EQUAL(static_cast<std::unique_ptr<long> *>(nullptr),
                      Si::try_get_ptr<std::unique_ptr<long>>(u));
    BOOST_CHECK_EQUAL(static_cast<std::unique_ptr<long> *>(nullptr),
                      Si::try_get_ptr<std::unique_ptr<long>>(v));
    BOOST_REQUIRE_NE(static_cast<std::unique_ptr<long> *>(nullptr),
                     Si::try_get_ptr<std::unique_ptr<long>>(w));
    BOOST_CHECK_EQUAL(123, *Si::try_get_ptr<int>(v));
    BOOST_CHECK_EQUAL(456, **Si::try_get_ptr<std::unique_ptr<long>>(w));
}

BOOST_AUTO_TEST_CASE(variant_const_try_get_ptr)
{
    Si::variant<Si::unit, int, std::unique_ptr<long>> const u, v(123),
        w(Si::make_unique<long>(456));
    BOOST_STATIC_ASSERT(
        (std::is_same<Si::unit const *,
                      decltype(Si::try_get_ptr<Si::unit>(u))>::value));
    BOOST_STATIC_ASSERT(
        (std::is_same<int const *, decltype(Si::try_get_ptr<int>(u))>::value));
    BOOST_STATIC_ASSERT(
        (std::is_same<std::unique_ptr<long> const *,
                      decltype(
                          Si::try_get_ptr<std::unique_ptr<long>>(u))>::value));
    BOOST_CHECK_NE(
        static_cast<Si::unit const *>(nullptr), Si::try_get_ptr<Si::unit>(u));
    BOOST_CHECK_EQUAL(
        static_cast<Si::unit const *>(nullptr), Si::try_get_ptr<Si::unit>(v));
    BOOST_CHECK_EQUAL(
        static_cast<Si::unit const *>(nullptr), Si::try_get_ptr<Si::unit>(w));
    BOOST_CHECK_EQUAL(
        static_cast<int const *>(nullptr), Si::try_get_ptr<int>(u));
    BOOST_REQUIRE_NE(
        static_cast<int const *>(nullptr), Si::try_get_ptr<int>(v));
    BOOST_CHECK_EQUAL(
        static_cast<int const *>(nullptr), Si::try_get_ptr<int>(w));
    BOOST_CHECK_EQUAL(static_cast<std::unique_ptr<long> const *>(nullptr),
                      Si::try_get_ptr<std::unique_ptr<long>>(u));
    BOOST_CHECK_EQUAL(static_cast<std::unique_ptr<long> const *>(nullptr),
                      Si::try_get_ptr<std::unique_ptr<long>>(v));
    BOOST_REQUIRE_NE(static_cast<std::unique_ptr<long> const *>(nullptr),
                     Si::try_get_ptr<std::unique_ptr<long>>(w));
    BOOST_CHECK_EQUAL(123, *Si::try_get_ptr<int>(v));
    BOOST_CHECK_EQUAL(456, **Si::try_get_ptr<std::unique_ptr<long>>(w));
}

BOOST_AUTO_TEST_CASE(variant_sizeof)
{
    BOOST_CHECK_EQUAL((sizeof(std::string) + sizeof(std::ptrdiff_t)),
                      sizeof(Si::variant<std::string>));
    BOOST_CHECK_EQUAL(
        sizeof(boost::uint8_t), sizeof(Si::variant<boost::uint8_t>));
    BOOST_CHECK_EQUAL(
        sizeof(boost::uint16_t), sizeof(Si::variant<boost::uint16_t>));
    BOOST_CHECK_EQUAL(
        sizeof(boost::uint32_t), sizeof(Si::variant<boost::uint32_t>));
    BOOST_CHECK_EQUAL(
        sizeof(boost::uint64_t), sizeof(Si::variant<boost::uint64_t>));
    BOOST_CHECK_EQUAL(sizeof(int *), sizeof(Si::variant<int *>));
    BOOST_CHECK_EQUAL(sizeof(std::hash<Si::variant<int>>),
                      sizeof(Si::variant<std::hash<Si::variant<int>>>));
    BOOST_CHECK_EQUAL((Si::alignment_of<int *>::value + sizeof(int *)),
                      sizeof(Si::variant<int *, int, Si::unit>));
    BOOST_CHECK_EQUAL(
        (Si::alignment_of<std::string>::value + sizeof(std::string)),
        sizeof(Si::variant<std::string, Si::unit>));
    BOOST_CHECK_EQUAL((sizeof(unsigned char) + sizeof(Si::unit)),
                      sizeof(Si::variant<Si::unit, Si::none_t>));
}

struct leaf
{
};

struct tree : Si::variant<leaf, std::vector<tree>>
{
    typedef Si::variant<leaf, std::vector<tree>> base;
};

BOOST_AUTO_TEST_CASE(variant_recursive)
{
    tree::base t;
    t = leaf();
    t = std::vector<tree>();
}

struct throws_on_move_exception : std::runtime_error
{
    throws_on_move_exception()
        : std::runtime_error("throws_on_move_exception")
    {
    }
};

#if SILICIUM_HAS_EXCEPTIONS
struct throws_on_move
{
    throws_on_move()
    {
    }

    throws_on_move(throws_on_move &&)
    {
        throw throws_on_move_exception();
    }

    SILICIUM_DELETED_FUNCTION(throws_on_move &operator=(throws_on_move &&))
};

struct null_visitor : boost::static_visitor<void>
{
    template <class T>
    void operator()(T &&) const
    {
    }
};

BOOST_AUTO_TEST_CASE(variant_move_throws)
{
    std::shared_ptr<int> content = std::make_shared<int>(123);
    std::weak_ptr<int> weak_content = content;
    Si::variant<std::shared_ptr<int>, throws_on_move> v = content;
    BOOST_CHECK_EQUAL(2, weak_content.use_count());
    content.reset();
    BOOST_CHECK_EQUAL(1, weak_content.use_count());
    BOOST_CHECK_EXCEPTION(v = throws_on_move(), throws_on_move_exception,
                          [](throws_on_move_exception const &)
                          {
                              return true;
                          });
    BOOST_CHECK_EQUAL(0, weak_content.use_count());
    BOOST_CHECK(v.valueless_by_exception());
    BOOST_CHECK_EXCEPTION(Si::apply_visitor(null_visitor(), v),
                          Si::bad_variant_access,
                          [](Si::bad_variant_access const &)
                          {
                              return true;
                          });
}
#endif

namespace
{
    struct add;
    struct reference;
    struct literal;

    // important: the elements are undefined types here
    typedef Si::variant<add, reference, literal> expression;

    struct add
    {
        std::unique_ptr<expression> left;
        std::unique_ptr<expression> right;
    };

    struct reference
    {
        std::string name;
    };

    struct literal
    {
        std::uint64_t value;
    };

    void print(std::ostream &out, expression const &root)
    {
        Si::visit<void>(root,
                        [&out](add const &add_)
                        {
                            out << '(';
                            print(out, *add_.left);
                            out << " + ";
                            print(out, *add_.right);
                            out << ')';
                        },
                        [&out](reference const &reference_)
                        {
                            out << reference_.name;
                        },
                        [&out](literal const &literal_)
                        {
                            out << literal_.value;
                        });
    }
}

BOOST_AUTO_TEST_CASE(variant_of_incomplete_types)
{
    std::unique_ptr<expression> right = Si::make_unique<expression>(
        add{Si::make_unique<expression>(reference{"b"}),
            Si::make_unique<expression>(literal{1})});
    expression root =
        add{Si::make_unique<expression>(reference{"a"}), std::move(right)};
    std::ostringstream buffer;
    print(buffer, root);
    BOOST_CHECK_EQUAL("(a + (b + 1))", buffer.str());
}

#define SILICIUM_SWITCH2(variant, first, second)                               \
    {                                                                          \
        switch ((variant).index())                                             \
        {                                                                      \
        case 0:                                                                \
        {                                                                      \
            auto &value = *Si::try_get<0>((variant));                          \
            first goto silicium_switch2_leave;                                 \
        }                                                                      \
        case 1:                                                                \
        {                                                                      \
            auto &value = *Si::try_get<1>((variant));                          \
            second goto silicium_switch2_leave;                                \
        }                                                                      \
        }                                                                      \
        SILICIUM_UNREACHABLE();                                                \
    silicium_switch2_leave:                                                    \
        (void)0;                                                               \
    }

#define SILICIUM_SWITCH2_BREAKABLE(variant, first, second)                     \
    {                                                                          \
        bool silicium_switch2_break_by_user = true;                            \
        switch ((variant).index())                                             \
        {                                                                      \
        case 0:                                                                \
        {                                                                      \
            auto &value = *Si::try_get<0>((variant));                          \
            first silicium_switch2_break_by_user = false;                      \
            break;                                                             \
        }                                                                      \
        case 1:                                                                \
        {                                                                      \
            auto &value = *Si::try_get<1>((variant));                          \
            second silicium_switch2_break_by_user = false;                     \
            break;                                                             \
        }                                                                      \
        }                                                                      \
        if (silicium_switch2_break_by_user)                                    \
        {                                                                      \
            break;                                                             \
        }                                                                      \
    }

BOOST_AUTO_TEST_CASE(variant_switch)
{
    Si::variant<int, std::string> v = 123;
    bool ok = false;
    SILICIUM_SWITCH2(v,
                     {
                         BOOST_REQUIRE_EQUAL(123, value);
                         BOOST_REQUIRE(!ok);
                         ok = true;
                     },
                     {
                         Si::ignore_unused_variable_warning(value);
                         BOOST_FAIL("wrong type");
                     })
    BOOST_CHECK(ok);
}

BOOST_AUTO_TEST_CASE(variant_switch_break)
{
    Si::variant<int, std::string> v = 123;
    bool ok = false;
    for (;;)
    {
        SILICIUM_SWITCH2_BREAKABLE(
            v,
            {
                BOOST_CHECK_EQUAL(123, value);
                BOOST_REQUIRE(!ok);
                ok = true;
                break;
            },
            {
                Si::ignore_unused_variable_warning(value);
                BOOST_FAIL("wrong type");
            })
        BOOST_FAIL("unreachable");
    }
    BOOST_CHECK(ok);
}
#endif
