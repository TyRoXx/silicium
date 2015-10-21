#ifndef SILICIUM_EXTENSIBLE_OBSERVABLE_HPP
#define SILICIUM_EXTENSIBLE_OBSERVABLE_HPP

#include <utility>
#include <silicium/observable/observer.hpp>

namespace Si
{
	template <class Element, class State>
	struct extensible_observer : observer<Element>
	{
		explicit extensible_observer(observer<Element> &stable_observer, State state)
		    : m_stable_observer(&stable_observer)
		    , m_state(std::move(state))
		{
		}

		observer<Element> *get() const
		{
			return m_stable_observer;
		}

		State const &state() const
		{
			return m_state;
		}

		void got_element(Element element) SILICIUM_OVERRIDE
		{
			m_stable_observer->got_element(std::move(element));
		}

		void ended() SILICIUM_OVERRIDE
		{
			m_stable_observer->ended();
		}

	private:
		observer<Element> *m_stable_observer;
		State m_state;
	};

	template <class Element, class State>
	auto make_extensible_observer(observer<Element> &stable_observer, State &&state)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> extensible_observer<Element, typename std::decay<State>::type>
#endif
	{
		return extensible_observer<Element, typename std::decay<State>::type>(stable_observer,
		                                                                      std::forward<State>(state));
	}

	template <class Element, class State, class OtherElement>
	auto extend(extensible_observer<Element, State> &&left, ptr_observer<observer<OtherElement>> right)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> decltype(make_extensible_observer(*right.get(), left.state()))
#endif
	{
		return make_extensible_observer(*right.get(), left.state());
	}
}

#endif
