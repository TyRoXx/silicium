#ifndef SILICIUM_RANGE_VALUE_HPP
#define SILICIUM_RANGE_VALUE_HPP

#include <silicium/config.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/functional/hash.hpp>

namespace Si
{
    template <class BidirectionalRange>
    struct range_value
    {
        BidirectionalRange range;

        range_value()
        {
        }

        range_value(BidirectionalRange range)
            : range(std::move(range))
        {
        }
    };

    template <class BidirectionalRange1, class BidirectionalRange2>
    bool operator==(range_value<BidirectionalRange1> const &left,
                    range_value<BidirectionalRange2> const &right)
    {
        return boost::range::equal(left.range, right.range);
    }

    template <class BidirectionalRange>
    auto make_range_value(BidirectionalRange &&range)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
        -> range_value<typename std::decay<BidirectionalRange>::type>
#endif
    {
        return range_value<typename std::decay<BidirectionalRange>::type>(
            std::forward<BidirectionalRange>(range));
    }
}

namespace std
{
    template <class BidirectionalRange>
    struct hash<Si::range_value<BidirectionalRange>>
    {
        std::size_t
        operator()(Si::range_value<BidirectionalRange> const &value) const
        {
            using boost::begin;
            using boost::end;
            return boost::hash_range(begin(value.range), end(value.range));
        }
    };
}

#endif
