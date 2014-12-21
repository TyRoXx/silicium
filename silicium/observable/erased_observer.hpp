#ifndef SILICIUM_ERASED_OBSERVER_HPP
#define SILICIUM_ERASED_OBSERVER_HPP

#include <silicium/observable/observer.hpp>

namespace Si
{
	template <class Element>
	struct erased_observer
	{
		typedef Element element_type;

		erased_observer() BOOST_NOEXCEPT
		{
		}

		template <class Observer>
		explicit erased_observer(Observer &&observer)
			: m_original(Si::to_unique(Si::virtualize_observer(std::forward<Observer>(observer))))
		{
		}

		erased_observer(erased_observer &&) BOOST_NOEXCEPT = default;
		erased_observer &operator = (erased_observer &&) BOOST_NOEXCEPT = default;

		void got_element(Element element)
		{
			assert(m_original);
			m_original->got_element(std::move(element));
		}

		void ended()
		{
			assert(m_original);
			m_original->ended();
		}

		observer<Element> *get() const BOOST_NOEXCEPT
		{
			return m_original.get();
		}

	private:

		std::unique_ptr<observer<Element>> m_original;
	};
}

#endif
