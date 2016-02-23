#ifndef SILICIUM_FUNCTION_SINK_HPP
#define SILICIUM_FUNCTION_SINK_HPP

#include <silicium/sink/sink.hpp>

namespace Si
{
	template <class Element, class Error, class Function>
	struct function_sink
	{
		typedef Element element_type;
		typedef Error error_type;

		function_sink()
		{
		}

		explicit function_sink(Function function)
		    : m_function(std::move(function))
		{
		}

		error_type append(iterator_range<element_type const *> data)
		{
			return m_function(data);
		}

	private:
		Function m_function;
	};

	template <class Element, class Function>
	auto make_function_sink(Function &&function)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> function_sink<Element, typename std::result_of<Function(
	                                  iterator_range<Element const *>)>::type,
	                     typename std::decay<Function>::type>
#endif
	{
		typedef typename std::result_of<Function(
		    iterator_range<Element const *>)>::type error;
		return function_sink<Element, error,
		                     typename std::decay<Function>::type>(
		    std::forward<Function>(function));
	}
}

#endif
