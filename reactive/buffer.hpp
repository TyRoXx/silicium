#ifndef SILICIUM_REACTIVE_BUFFER_HPP
#define SILICIUM_REACTIVE_BUFFER_HPP

#include <reactive/observable.hpp>

namespace rx
{
	template <class Element, class Original>
	struct buffer
			: public observable<Element>
			, private observer<Element>
	{
		typedef Element element_type;

		explicit buffer(Original from, std::size_t size)
			: from(std::move(from))
			, elements(size)
		{
			check_fetch();
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
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

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(receiver);
			assert(fetching);
			return from.cancel();
		}

	private:

		Original from;
		boost::circular_buffer<Element> elements;
		observer<element_type> *receiver = nullptr;
		bool fetching = false;

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
	};

	template <class Original>
	auto make_buffer(Original &&from, std::size_t size)
	{
		typedef typename std::decay<Original>::type clean_original;
		typedef typename clean_original::element_type element;
		return buffer<element, clean_original>(std::forward<Original>(from), size);
	}
}

#endif
