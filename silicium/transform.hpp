#ifndef SILICIUM_REACTIVE_TRANSFORM_HPP
#define SILICIUM_REACTIVE_TRANSFORM_HPP

#include <silicium/observable.hpp>
#include <silicium/config.hpp>
#include <silicium/override.hpp>
#include <type_traits>
#include <utility>
#include <cassert>

namespace Si
{
	template <class Transform, class Original>
	struct transformation
			: public observable<typename std::result_of<Transform (typename Original::element_type)>::type>
			, private observer<typename Original::element_type>
	{
		typedef typename std::result_of<Transform (typename Original::element_type)>::type element_type;
		typedef typename Original::element_type from_type;

		transformation()
		{
		}

		explicit transformation(Transform transform, Original original)
			: transform(std::move(transform))
			, original(std::move(original))
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

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!this->receiver);
			this->receiver = &receiver;
			original.async_get_one(*this);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			return original.cancel();
		}

	private:

		Transform transform; //TODO empty base optimization
		Original original;
		observer<element_type> *receiver = nullptr;

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

		BOOST_DELETED_FUNCTION(transformation(transformation const &))
		BOOST_DELETED_FUNCTION(transformation &operator = (transformation const &))
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
