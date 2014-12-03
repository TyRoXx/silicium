#ifndef SILICIUM_PTR_SINK_HPP
#define SILICIUM_PTR_SINK_HPP

#include <silicium/sink/sink.hpp>

namespace Si
{
	template <class Pointee, class Pointer>
	struct ptr_sink : sink<typename Pointee::element_type, typename Pointee::error_type>
	{
		typedef typename Pointee::element_type element_type;
		typedef typename Pointee::error_type error_type;

		ptr_sink()
			: next(nullptr)
		{
		}

		explicit ptr_sink(Pointer next)
			: next(std::move(next))
		{
		}

		virtual error_type append(iterator_range<element_type const *> data) SILICIUM_OVERRIDE
		{
			return next->append(data);
		}

	private:

		Pointer next;
	};

	template <class Pointee>
	auto ref_sink(Pointee &next)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> ptr_sink<Pointee, Pointee *>
#endif
	{
		return ptr_sink<Pointee, Pointee *>(&next);
	}
}

#endif
