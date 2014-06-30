#include <silicium/source.hpp>
#include <boost/test/unit_test.hpp>
#include <tuple>
#include <bitset>
#include <utility>
#include "integer_sequence.hpp"

namespace Si
{
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

		template <class Generated, class Element = typename std::result_of<Generated ()>::type>
		struct generator : observable<Element>
		{
			explicit generator(Generated generate)
				: generate(std::move(generate))
			{
			}

			virtual void async_get_one(observer<Element> &receiver) SILICIUM_OVERRIDE
			{
				return receiver.got_element(generate());
			}

			virtual void cancel() SILICIUM_OVERRIDE
			{
			}

		private:

			Generated generate;
		};

		template <class Generated>
		auto make_generator(Generated &&generate)
		{
			return generator<typename std::decay<Generated>::type>(std::forward<Generated>(generate));
		}

		template <class E>
		struct boxed_observable : observable<E>
		{
			explicit boxed_observable(std::unique_ptr<observable<E>> content)
				: content(std::move(content))
			{
			}

		private:

			std::unique_ptr<observable<E>> content;
		};

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
		auto and_(Parts &&...parts)
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
	}

	BOOST_AUTO_TEST_CASE(reactive_take)
	{
		auto zeros = rx::make_generator([]{ return 0; });
		auto ones  = rx::make_generator([]{ return 1; });
		auto both = rx::and_(zeros, ones);
		std::vector<std::tuple<int, int>> const expected(4, std::make_tuple(0, 1));
		std::vector<std::tuple<int, int>> const generated = rx::take(both, expected.size());
		BOOST_CHECK(expected == generated);
	}
}
