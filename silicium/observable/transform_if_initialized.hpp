#ifndef SILICIUM_REACTIVE_TRANSFORM_IF_INITIALIZED_HPP
#define SILICIUM_REACTIVE_TRANSFORM_IF_INITIALIZED_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/exchange.hpp>
#include <silicium/optional.hpp>
#include <cassert>
#include <stdexcept>
#include <boost/config.hpp>

namespace Si
{
	template <class Element, class Input, class Transformation, class Observer = ptr_observer<observer<Element>>>
	struct conditional_transformer
		: private observer<typename Input::element_type>
	{
		typedef Element element_type;

		conditional_transformer()
			: receiver_()
		{
		}

		explicit conditional_transformer(Input original, Transformation transform)
			: original(std::move(original))
			, transform(std::move(transform))
			, receiver_()
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
			original = std::move(other.original);
			transform = std::move(other.transform);
			receiver_ = std::move(other.receiver_);
			return *this;
		}
#endif

		void async_get_one(Observer receiver)
		{
			receiver_ = std::move(receiver);
			fetch();
		}

	private:

		Input original;
		Transformation transform; //TODO: optimize for emptiness
		optional<Observer> receiver_;

		SILICIUM_DELETED_FUNCTION(conditional_transformer(conditional_transformer const &))
		SILICIUM_DELETED_FUNCTION(conditional_transformer &operator = (conditional_transformer const &))

		void fetch()
		{
			original.async_get_one(observe_by_ref(static_cast<observer<typename Input::element_type> &>(*this)));
		}

		virtual void got_element(typename Input::element_type value) SILICIUM_OVERRIDE
		{
			auto converted = transform(std::move(value));
			if (converted)
			{
				std::move(*receiver_).got_element(std::move(*converted));
			}
			else
			{
				fetch();
			}
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			std::move(*receiver_).ended();
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
