#ifndef SILICIUM_REACTIVE_FLATTEN_HPP
#define SILICIUM_REACTIVE_FLATTEN_HPP

#include <silicium/observable.hpp>
#include <silicium/config.hpp>
#include <silicium/exchange.hpp>
#include <silicium/override.hpp>
#include <memory>
#include <unordered_map>
#include <cassert>
#include <boost/thread/locks.hpp>
#include <boost/range/algorithm/find_if.hpp>

namespace Si
{
	template <class NothingObservableObservable, class Mutex>
	struct flattener
			: public observable<nothing>
			, private observer<typename NothingObservableObservable::element_type>
	{
		typedef nothing element_type;

		flattener()
		{
		}

#ifdef _MSC_VER
		flattener(flattener &&other)
		{
			*this = std::move(other);
		}
		
		flattener &operator = (flattener &&other)
		{
			input = std::move(other.input);
			input_ended = std::move(other.input_ended);
			receiver_ = std::move(other.receiver_);
			children = std::move(other.children);
			children_mutex = std::move(other.children_mutex);
			return *this;
		}
#endif

		explicit flattener(NothingObservableObservable input)
			: input(std::move(input))
			, children_mutex(make_unique<Mutex>())
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			receiver_ = &receiver;
			fetch();
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			throw std::logic_error("to do");
		}

	private:

		typedef typename NothingObservableObservable::element_type nothing_observable;

		struct child
			: private observer<nothing>
			, private boost::noncopyable
		{
			flattener &parent;
			nothing_observable observed;

			explicit child(flattener &parent, nothing_observable observed)
				: parent(parent)
				, observed(std::move(observed))
			{
			}

			void start()
			{
				observed.async_get_one(*this);
			}

			virtual void got_element(nothing) SILICIUM_OVERRIDE
			{
				return start();
			}

			virtual void ended() SILICIUM_OVERRIDE
			{
				return parent.remove_child(*this);
			}
		};

		NothingObservableObservable input;
		bool input_ended = false;
		observer<nothing> *receiver_ = nullptr;
		std::unordered_map<child *, std::unique_ptr<child>> children;
		std::unique_ptr<Mutex> children_mutex;

		void fetch()
		{
			return input.async_get_one(*this);
		}

		void remove_child(child &removing)
		{
			boost::unique_lock<Mutex> lock(*children_mutex);
			auto const i = children.find(&removing);
			children.erase(i);
			if (input_ended &&
			    children.empty())
			{
				exchange(receiver_, nullptr)->ended();
			}
		}

		virtual void got_element(nothing_observable value) SILICIUM_OVERRIDE
		{
			{
				boost::unique_lock<Mutex> lock(*children_mutex);
				auto child_ = Si::make_unique<child>(*this, std::move(value));
				child &child_ref = *child_;
				children.insert(std::make_pair(&child_ref, std::move(child_)));
				child_ref.start();
			}
			return fetch();
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			boost::unique_lock<Mutex> lock(*children_mutex);
			assert(receiver_);
			input_ended = true;
			if (children.empty())
			{
				exchange(receiver_, nullptr)->ended();
			}
		}
	};

	template <class Mutex, class NothingObservableObservable>
	auto flatten(NothingObservableObservable &&input) -> flattener<typename std::decay<NothingObservableObservable>::type, Mutex>
	{
		return flattener<typename std::decay<NothingObservableObservable>::type, Mutex>(std::forward<NothingObservableObservable>(input));
	}
}

#endif
