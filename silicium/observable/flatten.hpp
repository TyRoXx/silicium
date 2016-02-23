#ifndef SILICIUM_REACTIVE_FLATTEN_HPP
#define SILICIUM_REACTIVE_FLATTEN_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/config.hpp>
#include <silicium/exchange.hpp>
#include <silicium/null_mutex.hpp>
#include <memory>
#include <unordered_map>
#include <cassert>
#include <boost/thread/locks.hpp>
#include <boost/range/algorithm/find_if.hpp>

namespace Si
{
	template <class ObservableObservable, class Mutex>
	struct flattener
	    : private observer<typename ObservableObservable::element_type>
	{
		typedef typename ObservableObservable::element_type sub_observable;
		typedef typename sub_observable::element_type element_type;

		flattener()
		    : input_ended(false)
		    , receiver_(nullptr)
		    , is_fetching(false)
		{
		}

#if !SILICIUM_COMPILER_GENERATES_MOVES
		flattener(flattener &&other)
		    : input_ended(false)
		    , receiver_(nullptr)
		    , is_fetching(false)
		{
			*this = std::move(other);
		}

		flattener &operator=(flattener &&other)
		{
			input = std::move(other.input);
			input_ended = std::move(other.input_ended);
			receiver_ = std::move(other.receiver_);
			children = std::move(other.children);
			children_mutex = std::move(other.children_mutex);
			is_fetching = other.is_fetching;
			return *this;
		}
#endif

		explicit flattener(ObservableObservable input)
		    : input(std::move(input))
		    , input_ended(false)
		    , receiver_(nullptr)
		    , children_mutex(make_unique<Mutex>())
		    , is_fetching(false)
		{
		}

		void async_get_one(ptr_observer<observer<element_type>> receiver)
		{
			assert(!receiver_);
			receiver_ = receiver.get();
			if (!is_fetching)
			{
				is_fetching = true;
				fetch();
			}
		}

	private:
		struct child : private observer<element_type>,
		               private boost::noncopyable
		{
			flattener &parent;
			sub_observable observed;

			explicit child(flattener &parent, sub_observable observed)
			    : parent(parent)
			    , observed(std::move(observed))
			{
			}

			void start()
			{
				observed.async_get_one(observe_by_ref(
				    static_cast<observer<element_type> &>(*this)));
			}

			virtual void got_element(element_type value) SILICIUM_OVERRIDE
			{
				Si::exchange(parent.receiver_, nullptr)
				    ->got_element(std::move(value));
				// TODO: fix the race condition between got_element and
				// async_get_one
				return start();
			}

			virtual void ended() SILICIUM_OVERRIDE
			{
				return parent.remove_child(*this);
			}
		};

		ObservableObservable input;
		bool input_ended;
		observer<element_type> *receiver_;
		std::unordered_map<child *, std::unique_ptr<child>> children;
		std::unique_ptr<Mutex> children_mutex;
		bool is_fetching;

		void fetch()
		{
			assert(is_fetching);
			return input.async_get_one(observe_by_ref(
			    static_cast<
			        observer<typename ObservableObservable::element_type> &>(
			        *this)));
		}

		void remove_child(child &removing)
		{
			boost::unique_lock<Mutex> lock(*children_mutex);
			auto const i = children.find(&removing);
			children.erase(i);
			if (input_ended && children.empty())
			{
				exchange(receiver_, nullptr)->ended();
			}
		}

		virtual void got_element(sub_observable value) SILICIUM_OVERRIDE
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

	template <class Mutex, class ObservableObservable>
	auto flatten(ObservableObservable &&input)
	    -> flattener<typename std::decay<ObservableObservable>::type, Mutex>
	{
		return flattener<typename std::decay<ObservableObservable>::type,
		                 Mutex>(std::forward<ObservableObservable>(input));
	}
}

#endif
