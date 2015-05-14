#ifndef SILICIUM_DETAIL_ARGUMENT_OF_HPP
#define SILICIUM_DETAIL_ARGUMENT_OF_HPP

#include <silicium/config.hpp>
#include <boost/static_assert.hpp>

namespace Si
{
	namespace detail
	{
		template <class Method>
		struct argument_of_method;

		template <class Result, class Class, class Argument>
		struct argument_of_method<Result(Class::*)(Argument) const>
		{
			typedef Argument type;
		};

		template <class Result, class Class, class Argument>
		struct argument_of_method<Result(Class::*)(Argument)>
		{
			typedef Argument type;
		};

		template <class Function>
		struct argument_of : argument_of_method<decltype(&Function::operator())>
		{
		};

		template <class Result, class Argument>
		struct argument_of<Result(*)(Argument)>
		{
			typedef Argument type;
		};

		BOOST_STATIC_ASSERT((std::is_same<int, argument_of<void(*)(int)>::type>::value));
	}
}

#endif
