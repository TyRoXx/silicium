#ifndef SILICIUM_TRANSFORMING_SOURCE_HPP
#define SILICIUM_TRANSFORMING_SOURCE_HPP

#include <silicium/iterator_range.hpp>
#include <silicium/source/source.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

namespace Si
{
    template <class From, class Transformation, class To>
    struct transforming_source
    {
        typedef To element_type;

        transforming_source()
        {
        }

        template <class Transformation2>
        explicit transforming_source(From original, Transformation2 &&transform)
            : original(std::move(original))
            , transform(std::forward<Transformation2>(transform))
        {
        }

        iterator_range<To const *> map_next(std::size_t size)
        {
            (void)size;
            return {};
        }

        To *copy_next(iterator_range<To *> destination)
        {
            auto i = boost::begin(destination);
            for (; i != boost::end(destination); ++i)
            {
                auto next = Si::get(original);
                if (!next)
                {
                    break;
                }
                *i = transform(std::move(*next));
            }
            return i;
        }

    private:
        From original;
        Transformation transform;
    };

    template <class From, class Transformation>
    auto make_transforming_source(From &&original, Transformation &&transform)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
        -> transforming_source<
            typename std::decay<From>::type,
            typename std::decay<Transformation>::type,
            decltype(transform(
                std::declval<typename std::decay<From>::type::element_type>()))>
#endif
    {
        return transforming_source<typename std::decay<From>::type,
                                   typename std::decay<Transformation>::type,
                                   decltype(transform(
                                       std::declval<typename std::decay<
                                           From>::type::element_type>()))>(
            std::forward<From>(original),
            std::forward<Transformation>(transform));
    }
}

#endif
