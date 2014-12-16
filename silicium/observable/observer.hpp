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

	template <class Observer>
	struct ptr_observer
	{
		ptr_observer()
			: m_impl(nullptr)
		{
		}

		explicit ptr_observer(Observer &impl)
			: m_impl(&impl)
		{
		}

		Observer *get() const
		{
			return m_impl;
		}

		void got_element(typename Observer::element_type element) const
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

		Observer *m_impl;
	};

	template <class Element>
	ptr_observer<observer<Element>> observe_by_ref(observer<Element> &ref)
	{
		return ptr_observer<observer<Element>>(ref);
	}
}

#endif
