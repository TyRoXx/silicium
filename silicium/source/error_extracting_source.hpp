#ifndef SILICIUM_ERROR_EXTRACTING_SOURCE_HPP
#define SILICIUM_ERROR_EXTRACTING_SOURCE_HPP

#include <silicium/source/source.hpp>

namespace Si
{
	namespace detail
	{
		template <class ErrorOr>
		struct get_error_parameters;

		template <class Value, class Error>
		struct get_error_parameters<error_or<Value, Error>>
		{
			typedef Error error_type;
			typedef Value value_type;
		};
	}

	template <class ErrorOrSource>
	struct error_extracting_source : source<typename detail::get_error_parameters<typename ErrorOrSource::element_type>::value_type>
	{
		typedef typename detail::get_error_parameters<typename ErrorOrSource::element_type>::value_type element_type;
		typedef typename detail::get_error_parameters<typename ErrorOrSource::element_type>::error_type error_type;

		error_extracting_source()
		{
		}

		explicit error_extracting_source(ErrorOrSource input)
			: m_input(std::move(input))
		{
		}

		virtual boost::iterator_range<element_type const *> map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			boost::ignore_unused_variable_warning(size);
			if (m_last_error)
			{
				return {};
			}
			auto original = m_input.map_next(1);
			if (original.empty())
			{
				return {};
			}
			auto &front = original.begin()[0];
			if (front.is_error())
			{
				m_last_error = front.error();
				return {};
			}
			BOOST_STATIC_ASSERT(std::is_reference<decltype(front.get())>::value);
			element_type const &value = front.get();
			return boost::make_iterator_range(&value, &value + 1);
		}

		virtual element_type *copy_next(boost::iterator_range<element_type *> destination) SILICIUM_OVERRIDE
		{
			element_type *copied = destination.begin();
			if (m_last_error)
			{
				return copied;
			}
			while (copied != destination.end())
			{
				auto element = Si::get(m_input);
				if (!element)
				{
					break;
				}
				if (element->is_error())
				{
					m_last_error = element->error();
					break;
				}
				*copied = std::move(*element).get();
				++copied;
			}
			return copied;
		}

		error_type const &get_last_error() const
		{
			return m_last_error;
		}

	private:

		ErrorOrSource m_input;
		error_type m_last_error;
	};

	template <class ErrorOrSource>
	auto make_error_extracting_source(ErrorOrSource &&input)
	{
		return error_extracting_source<typename std::decay<ErrorOrSource>::type>(std::forward<ErrorOrSource>(input));
	}
}

#endif
