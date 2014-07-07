#ifndef SILICIUM_REACTIVE_TRANSFORM_HPP
#define SILICIUM_REACTIVE_TRANSFORM_HPP

#include <reactive/observable.hpp>

namespace rx
{
	template <class Transform, class Original>
	struct transformation
			: public observable<typename std::result_of<Transform (typename Original::element_type)>::type>
			, private observer<typename Original::element_type>
	{
		typedef typename std::result_of<Transform (typename Original::element_type)>::type element_type;
		typedef typename Original::element_type from_type;

		explicit transformation(Transform transform, Original original)
			: transform(std::move(transform))
			, original(std::move(original))
		{
		}

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
