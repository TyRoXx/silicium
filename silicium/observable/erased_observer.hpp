#ifndef SILICIUM_ERASED_OBSERVER_HPP
#define SILICIUM_ERASED_OBSERVER_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/to_unique.hpp>

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
		    : m_original(to_unique(
		          virtualize_observer(std::forward<Observer>(observer))))
		{
		}

#if SILICIUM_COMPILER_GENERATES_MOVES
		erased_observer(erased_observer &&) BOOST_NOEXCEPT = default;
		erased_observer &operator=(erased_observer &&) BOOST_NOEXCEPT = default;
#else
		erased_observer(erased_observer &&other) BOOST_NOEXCEPT
		    : m_original(std::move(other.m_original))
		{
		}

		erased_observer &operator=(erased_observer &&other) BOOST_NOEXCEPT
		{
			m_original = std::move(other.m_original);
			return *this;
		}
#endif

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

		SILICIUM_DELETED_FUNCTION(erased_observer(erased_observer const &))
		SILICIUM_DELETED_FUNCTION(
		    erased_observer &operator=(erased_observer const &))
	};
}

#endif
