#ifndef SILICIUM_OBSERVABLE_SOURCE_HPP
#define SILICIUM_OBSERVABLE_SOURCE_HPP

#include <silicium/observable/observable.hpp>
#include <silicium/source/source.hpp>
#include <silicium/config.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

namespace Si
{
	template <class Observable, class YieldContext>
	struct observable_source
	{
		typedef typename Observable::element_type element_type;

		observable_source()
		    : yield(nullptr)
		{
		}

		observable_source(Observable input, YieldContext &yield)
		    : input(std::move(input))
		    , yield(&yield)
		{
		}

		iterator_range<element_type const *> map_next(std::size_t size)
		{
			(void)size;
			return {};
		}

		element_type *copy_next(iterator_range<element_type *> destination)
		{
			auto i = boost::begin(destination);
			for (; i != boost::end(destination); ++i)
			{
				assert(yield);
				auto element = yield->get_one(input);
				if (!element)
				{
					break;
				}
				*i = std::move(*element);
			}
			return i;
		}

		boost::uintmax_t minimum_size()
		{
			return 0;
		}

		optional<boost::uintmax_t> maximum_size()
		{
			return none;
		}

		std::size_t skip(std::size_t count)
		{
			size_t skipped = 0;
			assert(yield);
			while ((skipped < count) && yield->get_one(input))
			{
			}
			return skipped;
		}

	private:
		Observable input;
		YieldContext *yield;
	};

	template <class Observable, class YieldContext>
	auto make_observable_source(Observable &&input, YieldContext &yield)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> observable_source<typename std::decay<Observable>::type, YieldContext>
#endif
	{
		return observable_source<typename std::decay<Observable>::type, YieldContext>(std::forward<Observable>(input),
		                                                                              yield);
	}
}

#endif
