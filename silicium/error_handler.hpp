#ifndef SILICIUM_ERROR_HANDLER_HPP
#define SILICIUM_ERROR_HANDLER_HPP

#include <silicium/error_or.hpp>
#include <silicium/identity.hpp>

namespace Si
{
	namespace detail
	{
		struct returning_error_handler
		{
			BOOST_CONSTEXPR returning_error_handler()
			{
			}

			boost::system::error_code operator()(boost::system::error_code error, identity<void>) const
			{
				return error;
			}
		};

		struct throwing_error_handler
		{
			BOOST_CONSTEXPR throwing_error_handler()
			{
			}

			template <class Result>
			Result operator()(boost::system::error_code error, identity<Result>) const
			{
				throw_error(error);
			}

			void operator()(boost::system::error_code error, identity<void>) const
			{
				if (!error)
				{
					return;
				}
				throw_error(error);
			}
		};

		struct variant_error_handler
		{
			BOOST_CONSTEXPR variant_error_handler()
			{
			}

			template <class Result>
			error_or<Result> operator()(boost::system::error_code error, identity<Result>) const
			{
				return error;
			}
		};
	}

	static detail::returning_error_handler const return_;
	static detail::throwing_error_handler const throw_;
	static detail::variant_error_handler const variant_;
}

#endif
