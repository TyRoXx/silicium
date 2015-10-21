#ifndef SILICIUM_OBSERVABLE_ERROR_OR_ENUMERATE_HPP
#define SILICIUM_OBSERVABLE_ERROR_OR_ENUMERATE_HPP

#include <silicium/observable/enumerate.hpp>
#include <silicium/variant.hpp>
#include <silicium/error_or.hpp>
#include <queue>

#define SILICIUM_HAS_ERROR_OR_ENUMERATE SILICIUM_HAS_VARIANT

namespace Si
{
	namespace detail
	{
		template <class ErrorOrT>
		struct value_type_of_error_or;

		template <class T, class E>
		struct value_type_of_error_or<error_or<T, E>>
		{
			typedef T type;
		};

		template <class RangeObservable>
		struct error_or_enumerated_element
		{
			typedef typename value_type_of_error_or<typename RangeObservable::element_type>::type range_type;
			typedef typename std::decay<decltype(*boost::begin(std::declval<range_type>()))>::type type;
		};
	}

#if SILICIUM_HAS_ERROR_OR_ENUMERATE
	template <class RangeObservable>
	struct error_or_enumerator : private observer<typename RangeObservable::element_type>
	{
		typedef error_or<typename detail::error_or_enumerated_element<RangeObservable>::type> element_type;
		typedef error_or<typename RangeObservable::element_type> range_type;

		error_or_enumerator()
		    : receiver_(nullptr)
		{
		}

		explicit error_or_enumerator(RangeObservable input)
		    : input(std::move(input))
		    , receiver_(nullptr)
		{
		}

#if !SILICIUM_COMPILER_GENERATES_MOVES
		error_or_enumerator(error_or_enumerator &&other)
		    : input(std::move(other.input))
		    , buffered(std::move(other.buffered))
		    , receiver_(std::move(other.receiver_))
		{
		}

		error_or_enumerator &operator=(error_or_enumerator &&other)
		{
			input = std::move(other.input);
			buffered = std::move(other.buffered);
			receiver_ = std::move(other.receiver_);
			return *this;
		}
#else
		SILICIUM_DEFAULT_MOVE(error_or_enumerator)
#endif

		void async_get_one(ptr_observer<observer<element_type>> receiver)
		{
			assert(!receiver_);
			receiver_ = receiver.get();
			visit<void>(buffered,
			            [this](std::queue<element_type> const &elements)
			            {
				            if (elements.empty())
				            {
					            input.async_get_one(Si::observe_by_ref(
					                static_cast<observer<typename RangeObservable::element_type> &>(*this)));
				            }
				            else
				            {
					            pop();
				            }
				        },
			            [this](boost::system::error_code const &)
			            {
				            pop();
				        });
		}

	private:
		RangeObservable input;
		variant<std::queue<element_type>, boost::system::error_code> buffered;
		observer<element_type> *receiver_;

		virtual void got_element(typename RangeObservable::element_type value) SILICIUM_OVERRIDE
		{
			assert(receiver_);
			visit<void>(buffered,
			            [this, &value](std::queue<element_type> &elements)
			            {
				            if (value.is_error())
				            {
					            buffered = value.error();
					            pop();
				            }
				            else
				            {
					            for (auto &element : value.get())
					            {
						            elements.push(std::move(element));
					            }
					            if (!elements.empty())
					            {
						            pop();
					            }
				            }
				        },
			            [this](boost::system::error_code const &)
			            {
				            SILICIUM_UNREACHABLE();
				        });
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			assert(visit<bool>(buffered,
			                   [](std::queue<element_type> const &e)
			                   {
				                   return e.empty();
				               },
			                   [this](boost::system::error_code) -> bool
			                   {
				                   SILICIUM_UNREACHABLE();
				               }));
			exchange(receiver_, nullptr)->ended();
		}

		void pop()
		{
			visit<void>(buffered,
			            [this](std::queue<element_type> &elements)
			            {
				            auto element = std::move(elements.front());
				            elements.pop();
				            exchange(receiver_, nullptr)->got_element(std::move(element));
				        },
			            [this](boost::system::error_code const ec_copy)
			            {
				            buffered = std::queue<element_type>();
				            exchange(receiver_, nullptr)->got_element(ec_copy);
				        });
		}

		SILICIUM_DISABLE_COPY(error_or_enumerator)
	};

	template <class RangeObservable>
	auto error_or_enumerate(RangeObservable &&ranges) -> error_or_enumerator<typename std::decay<RangeObservable>::type>
	{
		return error_or_enumerator<typename std::decay<RangeObservable>::type>(std::forward<RangeObservable>(ranges));
	}
#endif
}

#endif
