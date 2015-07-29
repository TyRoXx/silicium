#ifndef SILICIUM_REACTIVE_RECEIVED_FROM_SOCKET_SOURCE_HPP
#define SILICIUM_REACTIVE_RECEIVED_FROM_SOCKET_SOURCE_HPP

#include <silicium/source/source.hpp>
#include <silicium/memory_range.hpp>
#include <silicium/error_or.hpp>
#include <boost/optional.hpp>
#include <boost/variant/static_visitor.hpp>
#include <cassert>

namespace Si
{
	namespace detail
	{
		inline boost::optional<memory_range> strip_error(error_or<memory_range> received)
		{
			return received.get_ptr() ? boost::optional<memory_range>(*received.get_ptr()) : boost::none;
		}
	}

	struct received_from_socket_source : Source<char>::interface
	{
		explicit received_from_socket_source(Source<error_or<memory_range>>::interface &original)
			: original(&original)
		{
		}

		memory_range buffered() const
		{
			return rest;
		}

		virtual iterator_range<char const *> map_next(std::size_t) SILICIUM_OVERRIDE
		{
			assert(original);
			if (rest.begin() == rest.end())
			{
				auto next = Si::get(*original);
				if (!next)
				{
					return {};
				}
				auto bytes = detail::strip_error(*next);
				if (!bytes)
				{
					return {};
				}
				rest = *bytes;
			}
			return iterator_range<char const *>(rest.begin(), rest.end());
		}

		virtual char *copy_next(iterator_range<char *> destination) SILICIUM_OVERRIDE
		{
			auto mapped = map_next(destination.size());
			auto const copy_size = std::min(destination.size(), mapped.size());
#ifdef _MSC_VER
			if (copy_size == 0)
			{
				//The VC++ 2013 copy_n requires non-nullptr iterators although nullptr is
				//a perfectly valid iterator in an empty range. We do not call copy_n in that special case.
				return destination.begin();
			}
#endif
			char * const copied = std::copy_n(mapped.begin(), copy_size, destination.begin());
			skip(std::distance(destination.begin(), copied));
			return copied;
		}

	private:

		Source<error_or<memory_range>>::interface *original;
		memory_range rest;

		std::size_t skip(std::size_t count)
		{
			std::size_t skipped = 0;
			auto const rest_size = rest.size();
			auto const rest_skipped = std::min<ptrdiff_t>(count, rest_size);
			rest.pop_front(rest_skipped);
			count -= rest_skipped;
			skipped += rest_skipped;
			if (count > 0)
			{
				//TODO
				SILICIUM_UNREACHABLE();
			}
			return skipped;
		}
	};
}

#endif
