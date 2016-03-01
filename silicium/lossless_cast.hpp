#ifndef SILICIUM_LOSSLESS_CAST_PP
#define SILICIUM_LOSSLESS_CAST_PP

#include <silicium/config.hpp>
#include <boost/static_assert.hpp>

namespace Si
{
	namespace detail
	{
		template <class To, class From>
		To lossless_cast(From original, std::true_type, std::true_type)
		{
			BOOST_STATIC_ASSERT(sizeof(To) >= sizeof(From));
			return original;
		}

		template <class To, class From>
		To lossless_cast(From original, std::false_type, std::false_type)
		{
			BOOST_STATIC_ASSERT(sizeof(To) >= sizeof(From));
			return original;
		}
	}

	template <class To, class From>
	To lossless_cast(From original)
	{
		return detail::template lossless_cast<To>(
		    original, typename std::is_signed<To>::type(),
		    typename std::is_signed<From>::type());
	}
}

#endif
