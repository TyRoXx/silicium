#ifndef SILICIUM_BUFFERING_SOURCE_HPP
#define SILICIUM_BUFFERING_SOURCE_HPP

#include <silicium/source/source.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace Si
{
	template <class Element>
	struct mutable_source : Source<Element>::interface
	{
		virtual iterator_range<Element *>
		map_next_mutable(std::size_t size) = 0;
		virtual std::size_t skip(std::size_t count) = 0;
	};

	template <class Next>
	struct buffering_source : mutable_source<typename Next::element_type>
	{
		typedef typename Next::element_type element_type;

		explicit buffering_source(Next &next, std::size_t capacity)
		    : m_next(&next)
		    , m_buffer(capacity)
		{
		}

		virtual iterator_range<element_type const *>
		map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			if (m_buffer.empty())
			{
				pull();
			}
			auto one = m_buffer.array_one();
			return make_iterator_range(
			    one.first, one.first + std::min(size, one.second));
		}

		virtual element_type *
		copy_next(iterator_range<element_type *> destination) SILICIUM_OVERRIDE
		{
			if (m_buffer.empty() &&
			    (static_cast<size_t>(destination.size()) < m_buffer.capacity()))
			{
				pull();
			}

			element_type *next = destination.begin();

			std::size_t taken_from_buffer = 0;
			for (auto b = m_buffer.begin();
			     (b != m_buffer.end()) && (next != destination.end());
			     ++next, ++b, ++taken_from_buffer)
			{
				*next = *b; // TODO move?
			}

			assert(m_next);
			element_type *const result =
			    m_next->copy_next(make_iterator_range(next, destination.end()));
			m_buffer.erase_begin(taken_from_buffer);
			return result;
		}

		virtual std::size_t skip(std::size_t count) SILICIUM_OVERRIDE
		{
			assert(m_next);
			auto skipped_buffer = std::min(count, m_buffer.size());
			m_buffer.erase_begin(skipped_buffer);
			return skipped_buffer;
		}

		virtual iterator_range<element_type *>
		map_next_mutable(std::size_t size) SILICIUM_OVERRIDE
		{
			if (m_buffer.empty())
			{
				pull();
			}
			auto one = m_buffer.array_one();
			return make_iterator_range(
			    one.first, one.first + std::min(size, one.second));
		}

	private:
		Next *m_next;
		boost::circular_buffer<element_type> m_buffer;

		void pull()
		{
			m_buffer.resize(m_buffer.capacity());
			auto one = m_buffer.array_one();
			assert(m_next);
			auto copied = m_next->copy_next(
			    make_iterator_range(one.first, one.first + one.second));
			std::size_t new_buffer_size =
			    static_cast<std::size_t>(std::distance(one.first, copied));
			if ((one.first + one.second) == copied)
			{
				auto two = m_buffer.array_two();
				assert(m_next);
				copied = m_next->copy_next(
				    make_iterator_range(two.first, two.first + two.second));
				new_buffer_size +=
				    static_cast<std::size_t>(std::distance(two.first, copied));
			}
			m_buffer.resize(new_buffer_size);
		}
	};

	template <class Source>
	buffering_source<Source> make_buffer(Source &buffered, std::size_t capacity)
	{
		return buffering_source<Source>(buffered, capacity);
	}

	template <class Element>
	struct mutable_source_iterator
	    : boost::iterator_facade<mutable_source_iterator<Element>, Element,
	                             std::input_iterator_tag>
	{
		mutable_source_iterator() BOOST_NOEXCEPT : m_source(nullptr)
		{
		}

		explicit mutable_source_iterator(Si::mutable_source<Element> &source)
		    : m_source(&source)
		{
			check_end();
		}

		Element &dereference() const
		{
			assert(m_source);
			assert(!equal(mutable_source_iterator()));
			auto element = m_source->map_next_mutable(1);
			assert(!element.empty());
			return element.front();
		}

		bool equal(mutable_source_iterator const &other) const BOOST_NOEXCEPT
		{
			assert(!m_source || !other.m_source ||
			       (m_source == other.m_source));
			return (m_source == other.m_source);
		}

		void increment()
		{
			assert(!equal(mutable_source_iterator()));
			assert(m_source);
			std::size_t skipped = m_source->skip(1);
			assert(skipped == 1);
			(void)skipped;
			check_end();
		}

	private:
		Si::mutable_source<Element> *m_source;

		void check_end()
		{
			if (m_source->map_next_mutable(1).empty())
			{
				// no elements in source -> go to end
				m_source = nullptr;
			}
		}
	};

	template <class Element>
	mutable_source_iterator<Element> begin(mutable_source<Element> &source)
	{
		return mutable_source_iterator<Element>(source);
	}

	template <class Element>
	mutable_source_iterator<Element> end(mutable_source<Element> &)
	{
		return mutable_source_iterator<Element>();
	}
}

#endif
