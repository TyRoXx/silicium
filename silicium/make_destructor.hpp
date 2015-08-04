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

		SILICIUM_DEFAULT_NOEXCEPT_MOVE(destructor)
		SILICIUM_DISABLE_COPY(destructor)

	private:

		optional<Callable> m_action;
	};

	template <class Callable>
	auto make_destructor(Callable &&action)
	{
		return destructor<typename std::decay<Callable>::type>(std::forward<Callable>(action));
	}
}

#endif
