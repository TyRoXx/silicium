#ifndef SILICIUM_REACTIVE_OBSERVER_HPP
#define SILICIUM_REACTIVE_OBSERVER_HPP

#include <utility>
#include <cassert>

namespace Si
{
	template <class Element>
	struct observer
	{
		typedef Element element_type;

		virtual ~observer()
		{
		}

		virtual void got_element(element_type value) = 0;
		virtual void ended() = 0;
	};

	template <class ObserverPtr>
	struct any_ptr_observer
	{
		typedef typename std::decay<decltype(*std::declval<ObserverPtr>())>::type observer_type;

		any_ptr_observer()
			: m_impl(ObserverPtr())
		{
		}

		explicit any_ptr_observer(ObserverPtr impl)
			: m_impl(std::move(impl))
		{
		}

		ObserverPtr const &get() const
		{
			return m_impl;
		}

		void got_element(typename observer_type::element_type element) const
		{
			assert(m_impl);
			m_impl->got_element(std::move(element));
		}

		void ended() const
		{
			assert(m_impl);
			m_impl->ended();
		}

	private:

		ObserverPtr m_impl;
	};

	template <class Element>
	using ptr_observer = any_ptr_observer<Element *>;

	template <class Element>
	ptr_observer<observer<Element>> observe_by_ref(observer<Element> &ref)
	{
		return ptr_observer<observer<Element>>(&ref);
	}
}

#endif
