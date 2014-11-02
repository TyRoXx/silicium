#ifndef SILICIUM_THROWING_SINK_HPP
#define SILICIUM_THROWING_SINK_HPP

#include <silicium/sink.hpp>
#include <silicium/config.hpp>

namespace Si
{
	template <class Next>
	struct throwing_sink : flushable_sink<typename Next::element_type, void>
	{
		typedef typename Next::element_type element_type;

		throwing_sink()
		{
		}

		explicit throwing_sink(Next next)
			: next(next)
		{
		}

		virtual boost::iterator_range<element_type *> make_append_space(std::size_t size) SILICIUM_OVERRIDE
		{
			return next.make_append_space(size);
		}

		virtual void flush_append_space() SILICIUM_OVERRIDE
		{
			auto error = next.flush_append_space();
			if (error)
			{
				throw boost::system::system_error(error);
			}
		}

		virtual void append(boost::iterator_range<element_type const *> data) SILICIUM_OVERRIDE
		{
			auto error = next.append(data);
			if (error)
			{
				throw boost::system::system_error(error);
			}
		}

		virtual void flush() SILICIUM_OVERRIDE
		{
			auto error = next.flush();
			if (error)
			{
				throw boost::system::system_error(error);
			}
		}

	private:

		Next next;
	};

	template <class Next>
	auto make_throwing_sink(Next &&next)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> throwing_sink<typename std::decay<Next>::type>
#endif
	{
		return throwing_sink<typename std::decay<Next>::type>(std::forward<Next>(next));
	}
}

#endif
