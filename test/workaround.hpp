#ifndef SILICIUM_TEST_WORKAROUND_HPP
#define SILICIUM_TEST_WORKAROUND_HPP

#include <boost/version.hpp>
#include <utility>
#include <ostream>

#if BOOST_VERSION >= 105900
// It seems that BOOST_CHECK_EQUAL_COLLECTIONS needs ostreamable elements
// since 1.59. We want to test collections of std::pair, so we have to
// define an ostream operator for pair.
namespace std
{
    template <class T, class U>
    ostream &operator<<(ostream &s, const pair<T, U> &p)
    {
        return s << '<' << p.first << ',' << p.second << '>';
    }
}
#endif

#endif
