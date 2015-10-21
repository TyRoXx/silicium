#ifndef SILICIUM_NULL_MUTEX_HPP
#define SILICIUM_NULL_MUTEX_HPP

namespace Si
{
	struct null_mutex
	{
		void lock()
		{
		}

		void unlock()
		{
		}
	};
}

#endif
