#ifndef SILICIUM_MAKE_DESTRUCTOR_HPP
#define SILICIUM_MAKE_DESTRUCTOR_HPP

#include <silicium/optional.hpp>

namespace Si
{
	template <class Callable>
	struct destructor
	{
		destructor()
		{
		}

		explicit destructor(Callable action)
		    : m_action(std::move(action))
		{
		}

		~destructor() BOOST_NOEXCEPT
		{
			if (!m_action)
			{
				return;
			}
			(*m_action)();
		}

#if SILICIUM_COMPILER_GENERATES_MOVES
		SILICIUM_DEFAULT_MOVE(destructor)
		SILICIUM_DISABLE_COPY(destructor)
#endif

	private:
		optional<Callable> m_action;
	};

	template <class Callable>
	auto make_destructor(Callable &&action)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> destructor<typename std::decay<Callable>::type>
#endif
	{
		return destructor<typename std::decay<Callable>::type>(std::forward<Callable>(action));
	}
}

#endif
