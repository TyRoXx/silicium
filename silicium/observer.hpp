#ifndef SILICIUM_REACTIVE_OBSERVER_HPP
#define SILICIUM_REACTIVE_OBSERVER_HPP

namespace rx
{
	template <class E>
	struct observer
	{
		typedef E element_type;

		virtual ~observer()
		{
		}

		virtual void got_element(element_type value) = 0;
		virtual void ended() = 0;
	};
}

#endif
