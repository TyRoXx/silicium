#ifndef SILICIUM_MEMORY_RANGE_HPP
#define SILICIUM_MEMORY_RANGE_HPP

#include <boost/range/iterator_range.hpp>

namespace Si
{
	typedef boost::iterator_range<char const *> memory_range;
	typedef boost::iterator_range<char *> mutable_memory_range;

	template <class To, class From>
	struct copy_const
	{
		typedef typename boost::mpl::if_<boost::is_const<From>, boost::add_const<To>, boost::remove_const<To>>::type::type type;
	};

	template <class Byte>
	auto make_memory_range(Byte *begin, Byte *end)
	{
		typedef typename copy_const<char, Byte>::type dest_type;
		return boost::make_iterator_range(
			reinterpret_cast<dest_type *>(begin),
			reinterpret_cast<dest_type *>(end)
		);
	}

	template <class ContiguousRange>
	auto make_memory_range(ContiguousRange &range)
	{
		using std::begin;
		using std::end;
		auto begin_ = begin(range);
		auto end_ = end(range);
		BOOST_STATIC_ASSERT(std::is_same<std::random_access_iterator_tag, typename std::iterator_traits<decltype(begin_)>::iterator_category>::value);
		return make_memory_range(boost::addressof(*begin_), boost::addressof(*end_));
	}
}

#endif
