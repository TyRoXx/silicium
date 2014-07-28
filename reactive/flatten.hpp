#ifndef SILICIUM_REACTIVE_FLATTEN_HPP
#define SILICIUM_REACTIVE_FLATTEN_HPP

#include <reactive/observable.hpp>
#include <reactive/config.hpp>
#include <reactive/exchange.hpp>
#include <silicium/override.hpp>
#include <vector>
#include <memory>
#include <cassert>
#include <boost/thread/locks.hpp>
#include <boost/range/algorithm/find_if.hpp>

namespace rx
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
				, observed(observed)
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
		std::vector<std::unique_ptr<child>> children;
		std::unique_ptr<Mutex> children_mutex;

		void fetch()
		{
			return input.async_get_one(*this);
		}

		void remove_child(child &removing)
		{
			boost::unique_lock<Mutex> lock(*children_mutex);
			auto const i = boost::range::find_if(children, [&removing](std::unique_ptr<child> const &element)
			{
				return element.get() == &removing;
			});
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
				children.emplace_back(rx::make_unique<child>(*this, std::move(value)));
				child &new_child = *children.back();
				new_child.start();
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
