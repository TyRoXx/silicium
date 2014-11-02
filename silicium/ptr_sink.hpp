#ifndef SILICIUM_PTR_SINK_HPP
#define SILICIUM_PTR_SINK_HPP

#include <silicium/sink.hpp>

namespace Si
{
	template <class Pointee>
	struct ptr_sink : flushable_sink<typename Pointee::element_type, boost::system::error_code>
	{
		typedef typename Pointee::element_type element_type;

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

		virtual boost::system::error_code flush_append_space() SILICIUM_OVERRIDE
		{
			return next->flush_append_space();
		}

		virtual boost::system::error_code append(boost::iterator_range<element_type const *> data) SILICIUM_OVERRIDE
		{
			return next->append(data);
		}

		virtual boost::system::error_code flush() SILICIUM_OVERRIDE
		{
			return next->flush();
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
