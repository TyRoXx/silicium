#ifndef SILICIUM_DETAIL_ARGUMENT_OF_HPP
#define SILICIUM_DETAIL_ARGUMENT_OF_HPP

#include <silicium/config.hpp>
#include <boost/static_assert.hpp>

namespace Si
{
	namespace detail
	{
#if SILICIUM_VC2012
		template <class Result, class Class, class Argument>
		Argument get_argument(Result (Class::*)(Argument));

		template <class Result, class Class, class Argument>
		Argument get_argument(Result (Class::*)(Argument) const);

		template <class Class>
		struct argument_of
		{
			typedef decltype(get_argument(&Class::operator())) type;
		};
#else
		template <class Method>
		struct argument_of_method;

		template <class Result, class Class, class Argument>
		struct argument_of_method<Result (Class::*)(Argument) const>
		{
			typedef Argument type;
		};

		template <class Result, class Class, class Argument>
		struct argument_of_method<Result (Class::*)(Argument)>
		{
			typedef Argument type;
		};

		template <class Function>
		struct argument_of : argument_of_method<decltype(&Function::operator())>
		{
		};
#endif
		template <class Result, class Argument>
		struct argument_of<Result (*)(Argument)>
		{
			typedef Argument type;
		};

		template <class Result, class Argument>
		struct argument_of<Result(&)(Argument)>
		{
			typedef Argument type;
		};

		struct test_class
		{
			void operator()(int);
		};

		struct test_class_const
		{
			void operator()(double) const;
		};

		BOOST_STATIC_ASSERT(
		    (std::is_same<int, argument_of<test_class>::type>::value));
		BOOST_STATIC_ASSERT(
		    (std::is_same<double, argument_of<test_class_const>::type>::value));
		BOOST_STATIC_ASSERT(
		    (std::is_same<int, argument_of<void (*)(int)>::type>::value));
	}
}

#endif
