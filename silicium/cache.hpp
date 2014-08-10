#ifndef SILICIUM_REACTIVE_CACHE_HPP
#define SILICIUM_REACTIVE_CACHE_HPP

#include <silicium/observable.hpp>
#include <boost/optional.hpp>
#include <silicium/override.hpp>

namespace Si
{
	template <class Input>
	struct cache_observable
			: public observable<typename Input::element_type>
			, private observer<typename Input::element_type>
	{
		typedef typename Input::element_type element_type;

		explicit cache_observable(Input input, boost::optional<element_type> cached)
			: input(std::move(input))
			, cached(std::move(cached))
		{
		}

#ifdef _MSC_VER
		cache_observable(cache_observable &&other)
			: input(std::move(other.input))
			, receiver_(std::move(other.receiver_))
			, cached(std::move(other.cached))
			, is_fetching(std::move(other.is_fetching))
		{
		}

		cache_observable &operator = (cache_observable &&other)
		{
			//TODO: exception safety
			input = std::move(other.input);
			receiver_ = std::move(other.receiver_);
			cached = std::move(other.cached);
			is_fetching = std::move(other.is_fetching);
			return *this;
		}
#endif

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			receiver_ = &receiver;
			if (cached)
			{
				exchange(receiver_, nullptr)->got_element(*cached);
				if (is_fetching)
				{
					return;
				}
			}

			is_fetching = true;
			input.async_get_one(*this);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			receiver_ = nullptr;
			input.cancel();
		}

	private:

		Input input;
		observer<element_type> *receiver_ = nullptr;
		boost::optional<element_type> cached;
		bool is_fetching = false;

		virtual void got_element(element_type value) SILICIUM_OVERRIDE
		{
			cached = std::move(value);
			if (receiver_)
			{
				exchange(receiver_, nullptr)->got_element(*cached);
			}
			input.async_get_one(*this);
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			cached.reset();
			if (receiver_)
			{
				exchange(receiver_, nullptr)->ended();
			}
		}
	};

	template <class Input>
	auto cache(Input &&input) -> cache_observable<typename std::decay<Input>::type>
	{
		return cache_observable<typename std::decay<Input>::type>(std::forward<Input>(input), boost::none);
	}

	template <class Input, class Cached>
	auto cache(Input &&input, Cached &&initially) -> cache_observable<typename std::decay<Input>::type>
	{
		return cache_observable<typename std::decay<Input>::type>(std::forward<Input>(input), std::forward<Cached>(initially));
	}
}

#endif
