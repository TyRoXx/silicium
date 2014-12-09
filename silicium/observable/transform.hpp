#ifndef SILICIUM_REACTIVE_TRANSFORM_HPP
#define SILICIUM_REACTIVE_TRANSFORM_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/config.hpp>
#include <silicium/detail/proper_value_function.hpp>
#include <type_traits>
#include <utility>
#include <cassert>
#include <functional>
#include <boost/config.hpp>

namespace Si
{
	template <class Transform, class Original>
	struct transformation
		: private observer<typename Original::element_type>
	{
		typedef typename std::result_of<Transform (typename Original::element_type)>::type element_type;
		typedef typename Original::element_type from_type;

		transformation()
			: receiver(nullptr)
		{
		}

		template <class Transform2>
		explicit transformation(Transform2 &&transform, Original original)
			: transform(std::forward<Transform2>(transform))
			, original(std::move(original))
			, receiver(nullptr)
		{
		}

#if !SILICIUM_COMPILER_GENERATES_MOVES
		transformation(transformation &&other)
			: transform(std::move(other.transform))
			, original(std::move(other.original))
			, receiver(std::move(other.receiver))
		{
		}

		transformation &operator = (transformation &&other)
		{
			//TODO: exception safety
			transform = std::move(other.transform);
			original = std::move(other.original);
			receiver = std::move(other.receiver);
			return *this;
		}
#else
		transformation(transformation &&other) = default;
		transformation &operator = (transformation &&other) = default;
#endif

		void async_get_one(observer<element_type> &receiver)
		{
			assert(!this->receiver);
			this->receiver = &receiver;
			original.async_get_one(*this);
		}

	private:

		typedef typename detail::proper_value_function<
			Transform,
			element_type,
			from_type
		>::type proper_transform;

		proper_transform transform; //TODO empty base optimization
		Original original;
		observer<element_type> *receiver;

		virtual void got_element(from_type value) SILICIUM_OVERRIDE
		{
			assert(receiver);
			auto *receiver_copy = receiver;
			receiver = nullptr;
			receiver_copy->got_element(transform(std::move(value)));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(receiver);
			auto *receiver_copy = receiver;
			receiver = nullptr;
			receiver_copy->ended();
		}

		SILICIUM_DELETED_FUNCTION(transformation(transformation const &))
		SILICIUM_DELETED_FUNCTION(transformation &operator = (transformation const &))
	};

	template <class Transform, class Original>
	auto transform(Original &&original, Transform &&transform) -> transformation<
		typename std::decay<Transform>::type,
		typename std::decay<Original>::type
	>
	{
		return transformation<
				typename std::decay<Transform>::type,
				typename std::decay<Original>::type
				>(std::forward<Transform>(transform), std::forward<Original>(original));
	}
}

#endif
