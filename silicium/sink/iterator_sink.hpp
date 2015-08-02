#ifndef SILICIUM_ITERATOR_SINK_HPP
#define SILICIUM_ITERATOR_SINK_HPP

#include <silicium/sink/sink.hpp>
#include <silicium/config.hpp>
#include <silicium/success.hpp>

namespace Si
{
	template <class Element, class OutputIterator>
	struct iterator_sink
	{
		typedef Element element_type;
		typedef success error_type;

		iterator_sink()
		{
		}

		explicit iterator_sink(OutputIterator out)
			: m_out(std::move(out))
		{
		}

#if SILICIUM_COMPILER_GENERATES_MOVES
		iterator_sink(iterator_sink &&other) = default;
		iterator_sink &operator = (iterator_sink &&other) = default;
		iterator_sink(iterator_sink const &other) = default;
		iterator_sink &operator = (iterator_sink const &other) = default;
#else
		iterator_sink(iterator_sink &&other)
			: m_out(std::move(other.m_out))
		{
		}

		iterator_sink &operator = (iterator_sink &&other)
		{
			m_out = std::move(other.m_out);
			return *this;
		}

		iterator_sink(iterator_sink const &other)
			: m_out(other.m_out)
		{
		}

		iterator_sink &operator = (iterator_sink const &other)
		{
			m_out = other.m_out;
			return *this;
		}
#endif

		success append(iterator_range<Element const *> data)
		{
			m_out = boost::range::copy(data, m_out);
			return success();
		}

		OutputIterator &position()
		{
			return m_out;
		}

		OutputIterator const &position() const
		{
			return m_out;
		}

	private:

		OutputIterator m_out;
	};

	template <class Element, class OutputIterator>
	auto make_iterator_sink(OutputIterator out)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> iterator_sink<Element, typename std::decay<OutputIterator>::type>
#endif
	{
		return iterator_sink<Element, typename std::decay<OutputIterator>::type>(std::move(out));
	}

	template <class Container>
	struct container_sink
	{
		typedef typename Container::value_type element_type;
		typedef success error_type;

		container_sink()
			: m_destination(nullptr)
		{
		}

		explicit container_sink(Container &destination)
			: m_destination(&destination)
		{
		}

		success append(iterator_range<element_type const *> data) const
		{
			assert(m_destination);
			m_destination->insert(m_destination->end(), data.begin(), data.end());
			return success();
		}

	private:

		Container *m_destination;
	};

	template <class Container>
	auto make_container_sink(Container &destination)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> container_sink<Container>
#endif
	{
		return container_sink<Container>(destination);
	}
}

#endif
