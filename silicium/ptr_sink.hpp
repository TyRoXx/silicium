#ifndef SILICIUM_PTR_SINK_HPP
#define SILICIUM_PTR_SINK_HPP

#include <silicium/sink.hpp>

namespace Si
{
	template <class Pointee>
	struct ptr_sink : sink<typename Pointee::element_type, typename Pointee::error_type>
	{
		typedef typename Pointee::element_type element_type;
		typedef typename Pointee::error_type error_type;

		ptr_sink()
			: next(nullptr)
		{
		}

		explicit ptr_sink(Pointee &next)
			: next(&next)
		{
		}

		virtual boost::iterator_range<element_type *> make_append_space(std::size_t size) SILICIUM_OVERRIDE
		{
			return next->make_append_space(size);
		}

		virtual error_type flush_append_space() SILICIUM_OVERRIDE
		{
			return next->flush_append_space();
		}

		virtual error_type append(boost::iterator_range<element_type const *> data) SILICIUM_OVERRIDE
		{
			return next->append(data);
		}

	private:

		Pointee *next;
	};

	template <class Pointee>
	auto ref_sink(Pointee &next)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> ptr_sink<Pointee>
#endif
	{
		return ptr_sink<Pointee>(next);
	}
}

#endif
