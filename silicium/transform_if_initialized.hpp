#ifndef SILICIUM_REACTIVE_TRANSFORM_IF_INITIALIZED_HPP
#define SILICIUM_REACTIVE_TRANSFORM_IF_INITIALIZED_HPP

#include <silicium/observer.hpp>
#include <silicium/exchange.hpp>
#include <silicium/override.hpp>
#include <silicium/config.hpp>
#include <cassert>
#include <boost/config.hpp>

namespace Si
{
	template <class Element, class Input, class Transformation>
	struct conditional_transformer
		: private observer<typename Input::element_type>
	{
		typedef Element element_type;

		conditional_transformer()
		{
		}

		explicit conditional_transformer(Input original, Transformation transform)
			: original(std::move(original))
			, transform(std::move(transform))
		{
		}

#if SILICIUM_COMPILER_GENERATES_MOVES
		conditional_transformer(conditional_transformer &&) = default;
		conditional_transformer &operator = (conditional_transformer &&) = default;
#else
		conditional_transformer(conditional_transformer &&other)
			: original(std::move(other.original))
			, transform(std::move(other.transform))
			, receiver_(std::move(other.receiver_))
		{
		}

		conditional_transformer &operator = (conditional_transformer &&other)
		{
			original = std:move(other.original);
			transform = std:move(other.transform);
			receiver_ = std:move(other.receiver_);
			return *this;
		}
#endif

		void async_get_one(observer<element_type> &receiver)
		{
			assert(!receiver_);
			receiver_ = &receiver;
			fetch();
		}

		void cancel()
		{
			throw std::logic_error("to do");
		}

	private:

		Input original;
		Transformation transform; //TODO: optimize for emptiness
		observer<element_type> *receiver_ = nullptr;

		BOOST_DELETED_FUNCTION(conditional_transformer(conditional_transformer const &))
		BOOST_DELETED_FUNCTION(conditional_transformer &operator = (conditional_transformer const &))

		void fetch()
		{
			original.async_get_one(*this);
		}

		virtual void got_element(typename Input::element_type value) SILICIUM_OVERRIDE
		{
			assert(receiver_);
			auto converted = transform(std::move(value));
			if (converted)
			{
				Si::exchange(receiver_, nullptr)->got_element(std::move(*converted));
			}
			else
			{
				fetch();
			}
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			SILICIUM_UNREACHABLE();
		}
	};

	template <class Element, class Input, class Transformation>
	auto transform_if_initialized(Input &&input, Transformation &&transform) -> conditional_transformer<Element, typename std::decay<Input>::type, typename std::decay<Transformation>::type>
	{
		return conditional_transformer<Element, typename std::decay<Input>::type, typename std::decay<Transformation>::type>(
			std::forward<Input>(input),
			std::forward<Transformation>(transform));
	}
}

#endif
