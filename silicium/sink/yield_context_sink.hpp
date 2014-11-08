#ifndef SILICIUM_YIELD_CONTEXT_SINK_HPP
#define SILICIUM_YIELD_CONTEXT_SINK_HPP

#include <silicium/sink.hpp>
#include <silicium/yield_context.hpp>

namespace Si
{
	template <class Element>
	struct yield_context_sink : Si::sink<Element>
	{
		explicit yield_context_sink(Si::push_context<Element> &yield)
			: yield(&yield)
		{
		}

		virtual boost::iterator_range<Element *> make_append_space(std::size_t size) SILICIUM_OVERRIDE
		{
			boost::ignore_unused_variable_warning(size);
			return {};
		}

		virtual void flush_append_space() SILICIUM_OVERRIDE
		{
		}

		virtual void append(boost::iterator_range<Element const *> data) SILICIUM_OVERRIDE
		{
			for (auto const &element : data)
			{
				(*yield)(element);
			}
		}

	private:

		Si::push_context<Element> *yield = nullptr;
	};
}

#endif
