#ifndef SILICIUM_SOURCE_HPP
#define SILICIUM_SOURCE_HPP

#include <silicium/override.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/optional.hpp>
#include <boost/cstdint.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <vector>

namespace Si
{
	template <class Element>
	struct source
	{
		virtual ~source()
		{
		}

		virtual boost::iterator_range<Element const *> map_next(std::size_t size) = 0;
		virtual Element *copy_next(boost::iterator_range<Element *> destination) = 0;
		virtual boost::uintmax_t minimum_size() = 0;
		virtual boost::optional<boost::uintmax_t> maximum_size() = 0;
		virtual std::size_t skip(std::size_t count) = 0;
	};

	template <class Element>
	struct mutable_source : source<Element>
	{
		virtual boost::iterator_range<Element *> map_next_mutable(std::size_t size) = 0;
	};

	template <class Element>
	struct memory_source SILICIUM_FINAL : source<Element>
	{
		memory_source()
		{
		}

		explicit memory_source(boost::iterator_range<Element const *> elements)
			: m_elements(std::move(elements))
		{
		}

		virtual boost::iterator_range<Element const *> map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			(void)size;
			return m_elements;
		}

		virtual Element *copy_next(boost::iterator_range<Element *> destination) SILICIUM_OVERRIDE
		{
			while (!m_elements.empty() && !destination.empty())
			{
				destination.front() = m_elements.front();
				destination.pop_front();
				m_elements.pop_front();
			}
			return destination.begin();
		}

		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE
		{
			return static_cast<boost::uintmax_t>(m_elements.size());
		}

		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE
		{
			return static_cast<boost::uintmax_t>(m_elements.size());
		}

		virtual std::size_t skip(std::size_t count) SILICIUM_OVERRIDE
		{
			auto skipped = std::min(count, static_cast<size_t>(m_elements.size()));
			m_elements.advance_begin(skipped);
			return skipped;
		}

	private:

		boost::iterator_range<Element const *> m_elements;
	};

	template <class Element>
	memory_source<Element> make_container_source(std::vector<Element> const &container)
	{
		return memory_source<Element>({container.data(), container.data() + container.size()});
	}

	template <class Element>
	memory_source<Element> make_container_source(std::basic_string<Element> const &container)
	{
		return memory_source<Element>({container.data(), container.data() + container.size()});
	}

	template <class Element>
	boost::optional<Element> get(Si::source<Element> &from)
	{
		Element result;
		if (&result == from.copy_next(boost::make_iterator_range(&result, &result + 1)))
		{
			return boost::none;
		}
		return std::move(result);
	}

	struct line_source SILICIUM_FINAL : Si::source<std::vector<char>>
	{
		line_source()
		{
		}

		explicit line_source(Si::source<char> &next)
			: m_next(&next)
		{
		}

		virtual boost::iterator_range<std::vector<char> const *> map_next(std::size_t) SILICIUM_OVERRIDE
		{
			return boost::iterator_range<std::vector<char> const *>();
		}

		virtual std::vector<char> *copy_next(boost::iterator_range<std::vector<char> *> destination) SILICIUM_OVERRIDE
		{
			assert(m_next);
			auto i = begin(destination);
			for (; i != end(destination); ++i)
			{
				auto &line = *i;
				for (;;)
				{
					auto c = get(*m_next);
					if (!c)
					{
						return &line;
					}
					if (*c == '\r')
					{
						auto lf = get(*m_next);
						if (!lf)
						{
							return &line;
						}
						if (*lf == '\n')
						{
							break;
						}
					}
					if (*c == '\n')
					{
						break;
					}
					line.emplace_back(*c);
				}
			}
			return i;
		}

		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE
		{
			return 0;
		}

		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE
		{
			assert(m_next);
			auto max_chars = m_next->maximum_size();
			//a line can be a single character ('\n')
			return max_chars;
		}

		virtual std::size_t skip(std::size_t count) SILICIUM_OVERRIDE
		{
			std::vector<char> thrown_away;
			for (size_t i = 0; i < count; ++i)
			{
				if (copy_next(boost::make_iterator_range(&thrown_away, &thrown_away + 1)) == &thrown_away)
				{
					return i;
				}
			}
			return count;
		}

	private:

		Si::source<char> *m_next = nullptr;
	};

	template <class Element>
	struct buffering_source SILICIUM_FINAL : mutable_source<Element>
	{
		explicit buffering_source(source<Element> &next, std::size_t capacity)
			: m_next(&next)
			, m_buffer(capacity)
		{
		}

		virtual boost::iterator_range<Element const *> map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			if (m_buffer.empty())
			{
				pull();
			}
			auto one = m_buffer.array_one();
			return boost::make_iterator_range(one.first, one.first + std::min(size, one.second));
		}

		virtual Element *copy_next(boost::iterator_range<Element *> destination) SILICIUM_OVERRIDE
		{
			if (m_buffer.empty() && (static_cast<size_t>(destination.size()) < m_buffer.capacity()))
			{
				pull();
			}

			Element *next = destination.begin();

			std::size_t taken_from_buffer = 0;
			for (auto b = m_buffer.begin(); (b != m_buffer.end()) && (next != destination.end()); ++next, ++b, ++taken_from_buffer)
			{
				*next = *b; //TODO move?
			}

			assert(m_next);
			Element * const result = m_next->copy_next(boost::make_iterator_range(next, destination.end()));
			m_buffer.erase_begin(taken_from_buffer);
			return result;
		}

		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE
		{
			assert(m_next);
			return m_buffer.size() + m_next->minimum_size();
		}

		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE
		{
			assert(m_next);
			auto next_max = m_next->maximum_size();
			if (next_max)
			{
				return (*next_max + m_buffer.size());
			}
			return boost::none;
		}

		virtual std::size_t skip(std::size_t count) SILICIUM_OVERRIDE
		{
			assert(m_next);
			auto skipped_buffer = std::min(count, m_buffer.size());
			m_buffer.erase_begin(skipped_buffer);
			auto rest = (count - skipped_buffer);
			return skipped_buffer + m_next->skip(rest);
		}

		virtual boost::iterator_range<Element *> map_next_mutable(std::size_t size) SILICIUM_OVERRIDE
		{
			if (m_buffer.empty())
			{
				pull();
			}
			auto one = m_buffer.array_one();
			return boost::make_iterator_range(one.first, one.first + std::min(size, one.second));
		}

	private:

		source<Element> *m_next = nullptr;
		boost::circular_buffer<Element> m_buffer;

		void pull()
		{
			m_buffer.resize(m_buffer.capacity());
			auto one = m_buffer.array_one();
			assert(m_next);
			auto copied = m_next->copy_next(boost::make_iterator_range(one.first, one.first + one.second));
			std::size_t new_buffer_size = std::distance(one.first, copied);
			if ((one.first + one.second) == copied)
			{
				auto two = m_buffer.array_two();
				assert(m_next);
				auto copied = m_next->copy_next(boost::make_iterator_range(two.first, two.first + two.second));
				new_buffer_size += std::distance(two.first, copied);
			}
			m_buffer.resize(new_buffer_size);
		}
	};

	template <class Element>
	buffering_source<Element> make_buffer(source<Element> &buffered, std::size_t capacity)
	{
		return buffering_source<Element>(buffered, capacity);
	}

	namespace detail
	{
		struct buffered_options
		{
			std::size_t capacity;
		};
	}

	inline detail::buffered_options buffered(std::size_t capacity)
	{
		return detail::buffered_options{capacity};
	}

	template <class Element>
	buffering_source<Element> operator | (source<Element> &buffered, detail::buffered_options options)
	{
		return make_buffer(buffered, options.capacity);
	}

	template <class Element>
	struct mutable_source_iterator SILICIUM_FINAL : boost::iterator_facade<mutable_source_iterator<Element>, Element, std::input_iterator_tag>
	{
		mutable_source_iterator() BOOST_NOEXCEPT
			: m_source(nullptr)
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
			assert(!m_source || !other.m_source || (m_source == other.m_source));
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
				//no elements in source -> go to end
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

	template <class Element, class Generator>
	struct generator_source SILICIUM_FINAL : source<Element>
	{
		explicit generator_source(Generator generate_next)
			: m_generate_next(std::move(generate_next))
		{
		}

		virtual boost::iterator_range<Element const *> map_next(std::size_t size) SILICIUM_OVERRIDE
		{
			(void)size;
			return boost::iterator_range<Element const *>();
		}

		virtual Element *copy_next(boost::iterator_range<Element *> destination) SILICIUM_OVERRIDE
		{
			auto copied = destination.begin();
			for (; copied != destination.end(); ++copied)
			{
				auto generated = m_generate_next();
				if (!generated)
				{
					break;
				}
				*copied = std::move(*generated);
			}
			return copied;
		}

		virtual boost::uintmax_t minimum_size() SILICIUM_OVERRIDE
		{
			return 0;
		}

		virtual boost::optional<boost::uintmax_t> maximum_size() SILICIUM_OVERRIDE
		{
			return boost::none;
		}

		virtual std::size_t skip(std::size_t count) SILICIUM_OVERRIDE
		{
			std::size_t i = 0;
			for (; i < count; ++i)
			{
				if (!m_generate_next())
				{
					break;
				}
			}
			return i;
		}

	private:

		Generator m_generate_next;
	};

	template <class Element, class Generator>
	generator_source<Element, typename std::decay<Generator>::type> make_generator_source(Generator &&generate_next)
	{
		return generator_source<Element, typename std::decay<Generator>::type>{std::forward<Generator>(generate_next)};
	}

	template <class To, class From, class Transformation>
	struct transforming_source SILICIUM_FINAL : source<To>
	{
		template <class Transformation2>
		explicit transforming_source(source<From> &original, Transformation2 &&transform)
			: original(original)
			, transform(std::forward<Transformation2>(transform))
		{
		}

		virtual boost::iterator_range<To const *> map_next(std::size_t size) override
		{
			(void)size;
			return {};
		}

		virtual To *copy_next(boost::iterator_range<To *> destination) override
		{
			auto i = boost::begin(destination);
			for (; i != boost::end(destination); ++i)
			{
				auto next = Si::get(original);
				if (!next)
				{
					break;
				}
				*i = transform(std::move(*next));
			}
			return i;
		}

		virtual boost::uintmax_t minimum_size() override
		{
			return original.minimum_size();
		}

		virtual boost::optional<boost::uintmax_t> maximum_size() override
		{
			return original.maximum_size();
		}

		virtual std::size_t skip(std::size_t count) override
		{
			return original.skip(count);
		}

	private:

		source<From> &original;
		Transformation const transform;
	};

	template <class To, class From, class Transformation>
	transforming_source<To, From, typename std::decay<Transformation>::type>
	make_transforming_source(source<From> &original, Transformation &&transform)
	{
		return transforming_source<To, From, typename std::decay<Transformation>::type>(original, std::forward<Transformation>(transform));
	}

	template <class Container>
	auto data(Container &container) -> decltype(&container[0])
	{
		return container.empty() ? nullptr : &container[0];
	}

	template <class Element, class Sequence = std::vector<Element>>
	auto take(source<Element> &from, std::size_t count) -> Sequence
	{
		Sequence taken;
		taken.resize(count);
		auto end = from.copy_next(boost::make_iterator_range(data(taken), data(taken) + taken.size()));
		taken.resize(std::distance(data(taken), end));
		return taken;
	}
}

#endif
