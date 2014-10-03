#ifndef SILICIUM_REACTIVE_RECEIVED_FROM_SOCKET_SOURCE_HPP
#define SILICIUM_REACTIVE_RECEIVED_FROM_SOCKET_SOURCE_HPP

#include <silicium/asio/socket_observable.hpp>
#include <silicium/source.hpp>
#include <boost/optional.hpp>
#include <boost/variant/static_visitor.hpp>
#include <cassert>

namespace Si
{
	namespace detail
	{
		inline boost::optional<Si::incoming_bytes> strip_error(Si::received_from_socket received)
		{
			return received.get_ptr() ? boost::optional<Si::incoming_bytes>(*received.get_ptr()) : boost::none;
		}
	}

	struct received_from_socket_source : Si::source<char>
	{
		explicit received_from_socket_source(Si::source<Si::received_from_socket> &original)
			: original(&original)
		{
		}

		Si::incoming_bytes buffered() const
		{
			return rest;
		}

		virtual boost::iterator_range<char const *> map_next(std::size_t) SILICIUM_OVERRIDE
		{
			assert(original);
			if (rest.begin == rest.end)
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
			return boost::iterator_range<char const *>(rest.begin, rest.end);
		}

		virtual char *copy_next(boost::iterator_range<char *> destination) SILICIUM_OVERRIDE
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

		Si::source<Si::received_from_socket> *original;
		Si::incoming_bytes rest;

		std::size_t skip(std::size_t count)
		{
			std::size_t skipped = 0;
			auto const rest_size = std::distance(rest.begin, rest.end);
			auto const rest_skipped = std::min<ptrdiff_t>(count, rest_size);
			rest.begin += rest_skipped;
			count -= rest_skipped;
			skipped += rest_skipped;
			if (count > 0)
			{
				throw std::logic_error("to do");
			}
			return skipped;
		}
	};
}

#endif
