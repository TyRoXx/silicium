#ifndef SILICIUM_REACTIVE_RECEIVED_FROM_SOCKET_SOURCE_HPP
#define SILICIUM_REACTIVE_RECEIVED_FROM_SOCKET_SOURCE_HPP

#include <silicium/socket_observable.hpp>
#include <silicium/source.hpp>
#include <boost/optional.hpp>
#include <boost/variant/static_visitor.hpp>
#include <cassert>

namespace rx
{
	namespace detail
	{
		struct error_stripper : boost::static_visitor<boost::optional<rx::incoming_bytes>>
		{
			boost::optional<rx::incoming_bytes> operator()(rx::incoming_bytes bytes) const
			{
				return bytes;
			}

			boost::optional<rx::incoming_bytes> operator()(boost::system::error_code) const
			{
				return boost::none;
			}
		};

		boost::optional<rx::incoming_bytes> strip_error(rx::received_from_socket received)
		{
			return Si::apply_visitor(error_stripper{}, received);
		}
	}

	struct received_from_socket_source : Si::source<char>
	{
		explicit received_from_socket_source(Si::source<rx::received_from_socket> &original)
			: original(&original)
		{
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

		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE
		{
			return std::distance(rest.begin, rest.end);
		}

		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE
		{
			return boost::none;
		}

		virtual std::size_t skip(std::size_t count) SILICIUM_OVERRIDE
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

	private:

		Si::source<rx::received_from_socket> *original = nullptr;
		rx::incoming_bytes rest;
	};
}

#endif
