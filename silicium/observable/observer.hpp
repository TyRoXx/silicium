#ifndef SILICIUM_REACTIVE_OBSERVER_HPP
#define SILICIUM_REACTIVE_OBSERVER_HPP

#include <utility>
#include <cassert>
#include <silicium/config.hpp>

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
		typedef typename observer_type::element_type element_type;

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

		void got_element(element_type element) const
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

#if SILICIUM_COMPILER_HAS_USING
	template <class Element>
	using ptr_observer = any_ptr_observer<Element *>;
#else
	template <class T>
	struct ptr_observer : any_ptr_observer<T *>
	{
		typedef typename any_ptr_observer<T *>::observer_type observer_type;
		typedef typename observer_type::element_type element_type;

		ptr_observer()
			: any_ptr_observer<T *>(nullptr)
		{
		}

		explicit ptr_observer(T *impl)
			: any_ptr_observer<T *>(impl)
		{
		}
	};
#endif

	template <class Element>
	ptr_observer<observer<Element>> observe_by_ref(observer<Element> &ref)
	{
		return ptr_observer<observer<Element>>(&ref);
	}

	template <class Original>
	struct virtualized_observer : observer<typename Original::element_type>
	{
		typedef typename Original::element_type element_type;

		virtualized_observer()
		{
		}

		explicit virtualized_observer(Original original)
			: m_original(std::move(original))
		{
		}

		virtual void got_element(element_type value) SILICIUM_OVERRIDE
		{
			return m_original.got_element(std::move(value));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			return m_original.ended();
		}

	private:

		Original m_original;
	};

	template <class Observer>
	auto virtualize_observer(Observer &&original)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> virtualized_observer<typename std::decay<Observer>::type>
#endif
	{
		return virtualized_observer<typename std::decay<Observer>::type>(std::forward<Observer>(original));
	}

	template <class Observer, class OtherObserver>
	auto extend(Observer &&, OtherObserver &&right)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
		-> OtherObserver
#endif
	{
		return std::forward<OtherObserver>(right);
	}
}

#endif
