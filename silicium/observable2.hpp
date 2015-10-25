#ifndef SILICIUM_OBSERVABLE2_HPP
#define SILICIUM_OBSERVABLE2_HPP

#include <silicium/function.hpp>
#include <silicium/trait.hpp>

namespace Si
{
	namespace observables2
	{
		template <class T>
		SILICIUM_TRAIT_WITH_TYPEDEFS(Observable, typedef T value_type;
		                             , ((observe, (1, (function<void(value_type)>)), void)))

		template <class T>
		struct never
		{
			typedef T value_type;

			template <class Handler>
			void observe(Handler &&handler)
			{
				assert(!m_handler);
				m_handler = std::forward<Handler>(handler);
				assert(m_handler);
			}

		private:
			function<void(T)> m_handler;
		};

		template <class T>
		struct pipe_shared
		{
			function<void(T)> handler;
		};

		template <class T>
		struct pipe_reader
		{
			typedef T value_type;
			explicit pipe_reader(std::shared_ptr<pipe_shared<T>> shared)
			    : m_shared(std::move(shared))
			{
			}

			template <class Handler>
			void observe(Handler &&handler)
			{
				assert(m_shared);
				m_shared->handler = std::forward<Handler>(handler);
			}

		private:
			std::shared_ptr<pipe_shared<T>> m_shared;
		};

		template <class T>
		struct pipe_writer
		{
			explicit pipe_writer(std::shared_ptr<pipe_shared<T>> shared)
			    : m_shared(std::move(shared))
			{
			}

			void push(T value)
			{
				assert(m_shared);
				assert(m_shared->handler);
				m_shared->handler(std::move(value));
			}

		private:
			std::shared_ptr<pipe_shared<T>> m_shared;
		};

		template <class T>
		std::pair<pipe_reader<T>, pipe_writer<T>> create_pipe()
		{
			std::shared_ptr<pipe_shared<T>> shared = std::make_shared<pipe_shared<T>>();
			pipe_reader<T> reader(shared);
			pipe_writer<T> writer(std::move(shared));
			return std::make_pair(std::move(reader), std::move(writer));
		}

		template <class Input, class Transformation>
		struct transformation
		{
			typedef typename Input::value_type input_value_type;
			typedef decltype(boost::declval<Transformation>()(boost::declval<input_value_type>())) value_type;

			transformation(Input input, Transformation transform)
			    : m_input(std::move(input))
			    , m_transform(std::move(transform))
			{
			}

			template <class Handler>
			void observe(Handler &&handler)
			{
				m_input.observe([ this, SILICIUM_CAPTURE_EXPRESSION(handler, std::forward<Handler>(handler)) ](
				    input_value_type value)
				                {
					                handler(m_transform(std::move(value)));
					            });
			}

		private:
			Input m_input;
			Transformation m_transform;
		};

		template <class Input, class Transformation>
		auto transform(Input &&input, Transformation &&transform)
		    -> transformation<typename std::decay<Input>::type, typename std::decay<Transformation>::type>
		{
			return transformation<typename std::decay<Input>::type, typename std::decay<Transformation>::type>(
			    std::forward<Input>(input), std::forward<Transformation>(transform));
		}
	}
}

#endif
