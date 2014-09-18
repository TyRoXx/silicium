#ifndef SILICIUM_REACTIVE_ENUMERATE_HPP
#define SILICIUM_REACTIVE_ENUMERATE_HPP

#include <silicium/observer.hpp>
#include <silicium/exchange.hpp>
#include <silicium/config.hpp>
#include <silicium/override.hpp>
#include <boost/range/begin.hpp>
#include <queue>
#include <stdexcept>
#include <cassert>

namespace Si
{
	template <class RangeObservable>
	struct enumerated_element
	{
		typedef typename std::decay<decltype(*boost::begin(std::declval<typename RangeObservable::element_type>()))>::type type;
	};

	template <class RangeObservable>
	struct enumerator
		: private observer<typename RangeObservable::element_type>
	{
		typedef typename enumerated_element<RangeObservable>::type element_type;
		typedef typename RangeObservable::element_type range_type;

		enumerator()
		{
		}

		explicit enumerator(RangeObservable input)
			: input(std::move(input))
		{
		}

#if !SILICIUM_COMPILER_GENERATES_MOVES
		enumerator(enumerator &&other)
			: input(std::move(other.input))
			, buffered(std::move(other.buffered))
			, receiver_(std::move(other.receiver_))
		{
		}

		enumerator &operator = (enumerator &&other)
		{
			input = std::move(other.input);
			buffered = std::move(other.buffered);
			receiver_ = std::move(other.receiver_);
			return *this;
		}
#else
		enumerator(enumerator &&) = default;
		enumerator &operator = (enumerator &&) = default;
#endif

		void async_get_one(observer<element_type> &receiver)
		{
			assert(!receiver_);
			receiver_ = &receiver;
			if (buffered.empty())
			{
				input.async_get_one(*this);
			}
			else
			{
				pop();
			}
		}

	private:

		RangeObservable input;
		std::queue<element_type> buffered;
		observer<element_type> *receiver_ = nullptr;

		virtual void got_element(range_type value) SILICIUM_OVERRIDE
		{
			assert(receiver_);
			for (auto &element : value)
			{
				buffered.push(std::move(element));
			}
			if (!buffered.empty())
			{
				pop();
			}
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			assert(buffered.empty());
			exchange(receiver_, nullptr)->ended();
		}

		void pop()
		{
			auto element = std::move(buffered.front());
			buffered.pop();
			return exchange(receiver_, nullptr)->got_element(std::move(element));
		}

		SILICIUM_DELETED_FUNCTION(enumerator(enumerator const &))
		SILICIUM_DELETED_FUNCTION(enumerator &operator = (enumerator const &))
	};

	template <class RangeObservable>
	auto enumerate(RangeObservable &&ranges) -> enumerator<typename std::decay<RangeObservable>::type>
	{
		return enumerator<typename std::decay<RangeObservable>::type>(std::forward<RangeObservable>(ranges));
	}
}

#endif
