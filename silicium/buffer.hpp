#ifndef SILICIUM_BUFFER_HPP
#define SILICIUM_BUFFER_HPP

#include <silicium/override.hpp>
#include <silicium/observable/observer.hpp>
#include <silicium/config.hpp>
#include <boost/circular_buffer.hpp>
#include <cassert>
#include <cstddef>

namespace Si
{
	template <class Element, class Original>
	struct buffer_observable
		: private observer<Element>
	{
		typedef Element element_type;

		buffer_observable()
			: receiver(nullptr)
			, fetching(false)
		{
		}

		explicit buffer_observable(Original from, std::size_t size)
			: from(std::move(from))
			, elements(size)
			, receiver(nullptr)
			, fetching(false)
		{
		}

#if defined(_MSC_VER) || (BOOST_VERSION <= 105400)
		buffer_observable(buffer_observable &&other)
			: from(std::move(other.from))
			, elements(std::move(other.elements))
			, receiver(nullptr)
			, fetching(false)
		{
		}

		buffer_observable &operator = (buffer_observable &&other)
		{
			//TODO: exception safety
			from = std::move(other.from);
			elements = std::move(other.elements);
			receiver = std::move(other.receiver);
			fetching = std::move(other.fetching);
			return *this;
		}
#else
		buffer_observable(buffer_observable &&) = default;
		buffer_observable &operator = (buffer_observable &&) = default;
#endif

		void async_get_one(observer<element_type> &receiver)
		{
			assert(!this->receiver);
			this->receiver = &receiver;
			if (elements.empty())
			{
				return check_fetch();
			}
			else
			{
				return deliver_front();
			}
		}

		void prefetch()
		{
			check_fetch();
		}

	private:

		Original from;
		boost::circular_buffer<Element> elements;
		observer<element_type> *receiver;
		bool fetching;

		virtual void got_element(element_type value) SILICIUM_OVERRIDE
		{
			assert(!elements.full());
			assert(fetching);
			fetching = false;
			if (elements.empty() &&
				receiver)
			{
				exchange(receiver, nullptr)->got_element(std::move(value));
				return check_fetch();
			}
			elements.push_back(std::move(value));
			if (!receiver)
			{
				return check_fetch();
			}
			deliver_front();
			return check_fetch();
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(fetching);
			assert(receiver);
			exchange(receiver, nullptr)->ended();
		}

		void deliver_front()
		{
			auto front = std::move(elements.front());
			elements.pop_front();
			exchange(receiver, nullptr)->got_element(std::move(front));
		}

		void check_fetch()
		{
			if (elements.full())
			{
				return;
			}
			if (fetching)
			{
				return;
			}
			fetching = true;
			from.async_get_one(*this);
		}

		SILICIUM_DELETED_FUNCTION(buffer_observable(buffer_observable const &))
		SILICIUM_DELETED_FUNCTION(buffer_observable &operator = (buffer_observable const &))
	};

	template <class Original>
	auto make_buffer_observable(Original &&from, std::size_t size) -> buffer_observable<typename std::decay<Original>::type::element_type, typename std::decay<Original>::type>
	{
		typedef typename std::decay<Original>::type clean_original;
		typedef typename clean_original::element_type element;
		return buffer_observable<element, clean_original>(std::forward<Original>(from), size);
	}
}

#endif
