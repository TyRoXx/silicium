#ifndef SILICIUM_NOEXCEPT_STRING_HPP
#define SILICIUM_NOEXCEPT_STRING_HPP

#include <silicium/config.hpp>
#include <string>
#include <boost/container/string.hpp>

namespace Si
{
#ifdef _MSC_VER
    // boost string does not work at all on VC++ 2013 Update 3, so we use
    // std::string instead
    typedef std::string noexcept_string;

    inline noexcept_string &&to_noexcept_string(noexcept_string &&str)
    {
        return std::move(str);
    }

    inline noexcept_string to_noexcept_string(noexcept_string const &str)
    {
        return str;
    }
#else

#if defined(__GNUC__) &&                                                       \
    (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7)) || (__GNUC__ >= 5))
    typedef std::string noexcept_string;

    inline noexcept_string
    to_noexcept_string(boost::container::string const &str)
    {
        return noexcept_string(str.data(), str.size());
    }
#else
    typedef boost::container::string noexcept_string;

    inline noexcept_string to_noexcept_string(std::string const &str)
    {
        return noexcept_string(str.data(), str.size());
    }
#endif
    inline noexcept_string &&to_noexcept_string(noexcept_string &&str)
    {
        return std::move(str);
    }

    inline noexcept_string to_noexcept_string(noexcept_string const &str)
    {
        return str;
    }
#endif

    template <class String = noexcept_string>
    String to_utf8_string(char const *utf8)
    {
        return utf8;
    }

    namespace detail
    {
        template <class Target, class Original>
        Target convert_range_impl(Original const &original, std::false_type)
        {
            using std::begin;
            using std::end;
            return Target(begin(original), end(original));
        }

        template <class Target, class Original>
        Target const &convert_range_impl(Original const &original,
                                         std::true_type)
        {
            return original;
        }

        template <class Target, class Original>
        auto convert_range(Original const &original) ->
#if SILICIUM_COMPILER_HAS_DECLTYPE_AUTO
            decltype(auto)
#else
            decltype(convert_range_impl<Target>(
                original, typename std::is_same<Target, Original>::type()))
#endif
        {
            return convert_range_impl<Target>(
                original, typename std::is_same<Target, Original>::type());
        }
    }

    template <class String = noexcept_string>
    String to_utf8_string(std::string const &utf8)
    {
#ifdef _WIN32
        return utf8;
#else
        return detail::convert_range<String>(utf8);
#endif
    }

    template <class String = noexcept_string>
    String to_utf8_string(boost::container::string const &utf8)
    {
        return detail::convert_range<String>(utf8);
    }
}

#endif
