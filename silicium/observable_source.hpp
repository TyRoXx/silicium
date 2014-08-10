#ifndef SILICIUM_REACTIVE_OBSERVABLE_SOURCE_HPP
#define SILICIUM_REACTIVE_OBSERVABLE_SOURCE_HPP

#include <silicium/observable.hpp>
#include <silicium/source.hpp>

namespace rx
{
	template <class Observable, class YieldContext>
	struct observable_source : Si::source<typename Observable::element_type>
	{
		typedef typename Observable::element_type element_type;

		observable_source(Observable input, YieldContext &yield)
			: input(std::move(input))
			, yield(&yield)
		{
		}

		virtual boost::iterator_range<element_type const *> map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			(void)size;
			return {};
		}

		virtual element_type *copy_next(boost::iterator_range<element_type *> destination) SILICIUM_OVERRIDE
		{
			using boost::begin;
			using boost::end;
			auto i = begin(destination);
			for (; i != end(destination); ++i)
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

		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE
		{
			return 0;
		}

		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE
		{
			return boost::none;
		}

		virtual std::size_t skip(std::size_t count) SILICIUM_OVERRIDE
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
		YieldContext *yield = nullptr;
	};

	template <class Observable, class YieldContext>
	auto make_observable_source(Observable &&input, YieldContext &yield) -> observable_source<typename std::decay<Observable>::type, YieldContext>
	{
		return observable_source<typename std::decay<Observable>::type, YieldContext>(std::forward<Observable>(input), yield);
	}
}

#endif
