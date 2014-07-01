#ifndef SILICIUM_REACTIVE_OBSERVABLE_HPP
#define SILICIUM_REACTIVE_OBSERVABLE_HPP

#include <silicium/override.hpp>
#include "detail/integer_sequence.hpp"
#include <tuple>
#include <bitset>
#include <utility>
#include <boost/circular_buffer.hpp>
#include <boost/optional.hpp>

namespace rx
{
	template <class E>
	struct observer
	{
		typedef E element_type;

		virtual ~observer()
		{
		}

		virtual void got_element(element_type value) = 0;
		virtual void ended() = 0;
	};

	template <class E>
	struct observable
	{
		typedef E element_type;

		virtual ~observable()
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) = 0;
		virtual void cancel() = 0;
	};

	template <class Element, class Ptr>
	struct ptr_observable : observable<Element>
	{
		typedef Element element_type;

		explicit ptr_observable(Ptr content)
			: content(std::move(content))
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(content);
			return content->async_get_one(receiver);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(content);
			return content->cancel();
		}

	private:

		Ptr content;
	};

	template <class Element, class Content>
	auto box(Content &&content)
	{
		return ptr_observable<Element, std::unique_ptr<observable<Element>>>(std::unique_ptr<observable<Element>>(new typename std::decay<Content>::type(std::forward<Content>(content))));
	}

	template <class Element, class Content>
	auto wrap(Content &&content)
	{
		return ptr_observable<Element, std::shared_ptr<observable<Element>>>(std::make_shared<typename std::decay<Content>::type>(std::forward<Content>(content)));
	}

	template <class ...Parts>
	struct and_combinator : observable<std::tuple<typename Parts::element_type...>>
	{
		typedef std::tuple<Parts...> parts_tuple;
		typedef std::tuple<typename Parts::element_type...> buffer_tuple;

		template <class PartsTuple>
		explicit and_combinator(PartsTuple &&parts)
			: parts(std::forward<PartsTuple>(parts))
		{
		}

		virtual void async_get_one(observer<buffer_tuple> &receiver) SILICIUM_OVERRIDE
		{
			assert(!this->receiver);
			this->receiver = &receiver;
			this->elements_received.reset();
			return async_get_one_impl<0, Parts...>();
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			return cancel_impl<0, Parts...>();
		}

	private:

		template <class Element, std::size_t Index>
		struct tuple_observer : observer<Element>
		{
			and_combinator *combinator = nullptr;

			virtual void got_element(Element value) SILICIUM_OVERRIDE
			{
				std::get<Index>(combinator->buffer) = std::move(value);
				combinator->elements_received.set(Index);
				combinator->check_received();
			}

			virtual void ended() SILICIUM_OVERRIDE
			{
				auto *receiver_copy = combinator->receiver;
				combinator->receiver = nullptr;
				receiver_copy->ended();
			}
		};

		template <class Indices>
		struct make_observers;

		template <std::size_t ...I>
		struct make_observers<ranges::v3::integer_sequence<I...>>
		{
			typedef std::tuple<tuple_observer<typename Parts::element_type, I>...> type;
		};

		typedef typename make_observers<typename ranges::v3::make_integer_sequence<sizeof...(Parts)>::type>::type observers_type;

		parts_tuple parts;
		observer<buffer_tuple> *receiver = nullptr;
		buffer_tuple buffer;
		std::bitset<sizeof...(Parts)> elements_received;
		observers_type observers;

		template <std::size_t Index, class Head, class ...Tail>
		void async_get_one_impl()
		{
			auto &observer = std::get<Index>(observers);
			observer.combinator = this;
			auto &part = std::get<Index>(parts);
			part.async_get_one(observer);
			return async_get_one_impl<Index + 1, Tail...>();
		}

		template <std::size_t Index>
		void async_get_one_impl()
		{
		}

		template <std::size_t Index, class Head, class ...Tail>
		void cancel_impl()
		{
			assert(receiver);
			auto &part = std::get<Index>(parts);
			part.cancel();
			return cancel_impl<Index + 1, Tail...>();
		}

		template <std::size_t Index>
		void cancel_impl()
		{
		}

		void check_received()
		{
			if (!elements_received.all())
			{
				return;
			}

			auto *receiver_copy = receiver;
			receiver = nullptr;
			receiver_copy->got_element(std::move(buffer));
		}
	};

	template <class ...Parts>
	auto make_tuple(Parts &&...parts)
	{
		return and_combinator<typename std::decay<Parts>::type...>(std::make_tuple(std::forward<Parts>(parts)...));
	}

	template <class Element>
	struct optional_observer : observer<Element>
	{
		boost::optional<Element> element;

		virtual void got_element(Element value) SILICIUM_OVERRIDE
		{
			element = std::move(value);
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			element.reset();
		}
	};

	template <class Element>
	boost::optional<Element> get_immediate(observable<Element> &from)
	{
		optional_observer<Element> current;
		from.async_get_one(current);
		return std::move(current.element);
	}

	template <class Element>
	std::vector<Element> take(observable<Element> &from, std::size_t count)
	{
		std::vector<Element> taken;
		for (std::size_t i = 0; i < count; ++i)
		{
			auto current = get_immediate(from);
			if (!current)
			{
				break;
			}
			taken.emplace_back(std::move(*current));
		}
		return taken;
	}

	template <class Transform, class Original>
	struct transformation
			: public observable<typename std::result_of<Transform (typename Original::element_type)>::type>
			, private observer<typename Original::element_type>
	{
		typedef typename std::result_of<Transform (typename Original::element_type)>::type element_type;
		typedef typename Original::element_type from_type;

		explicit transformation(Transform transform, Original original)
			: transform(std::move(transform))
			, original(std::move(original))
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!this->receiver);
			this->receiver = &receiver;
			original.async_get_one(*this);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			return original.cancel();
		}

	private:

		Transform transform; //TODO empty base optimization
		Original original;
		observer<element_type> *receiver = nullptr;

		virtual void got_element(from_type value) SILICIUM_OVERRIDE
		{
			assert(receiver);
			auto *receiver_copy = receiver;
			receiver = nullptr;
			receiver_copy->got_element(transform(std::move(value)));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(receiver);
			auto *receiver_copy = receiver;
			receiver = nullptr;
			receiver_copy->ended();
		}
	};

	template <class Transform, class Original>
	auto transform(Original &&original, Transform &&transform)
	{
		return transformation<
				typename std::decay<Transform>::type,
				typename std::decay<Original>::type
				>(std::forward<Transform>(transform), std::forward<Original>(original));
	}

	template <class T, class U>
	T exchange(T &dest, U &&source)
	{
		auto old = dest;
		dest = std::forward<U>(source);
		return old;
	}

	template <class Element>
	struct bridge : observable<Element>, observer<Element>
	{
		typedef Element element_type;

		bool is_waiting() const
		{
			return receiver != nullptr;
		}

		virtual void got_element(element_type value) SILICIUM_OVERRIDE
		{
			assert(receiver);
			exchange(receiver, nullptr)->got_element(std::move(value));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(receiver);
			exchange(receiver, nullptr)->ended();
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!this->receiver);
			this->receiver = &receiver;
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(receiver);
			receiver = nullptr;
		}

	private:

		observer<element_type> *receiver = nullptr;
	};

	template <class Element, class Consume>
	struct consumer : observer<Element>
	{
		typedef Element element_type;

		explicit consumer(Consume consume)
			: consume(std::move(consume))
		{
		}

		virtual void got_element(element_type value) SILICIUM_OVERRIDE
		{
			consume(std::move(value));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
		}

	private:

		Consume consume;
	};

	template <class Element, class Consume>
	auto consume(Consume con)
	{
		return consumer<Element, typename std::decay<Consume>::type>(std::move(con));
	}
}

#endif
