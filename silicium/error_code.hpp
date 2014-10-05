#ifndef SILICIUM_ERROR_CODE_HPP
#define SILICIUM_ERROR_CODE_HPP

#include <silicium/config.hpp>
#include <silicium/override.hpp>
#include <boost/system/error_code.hpp>
#include <boost/throw_exception.hpp>
#include <boost/static_assert.hpp>

namespace Si
{
	template <class UnderlyingErrorCode = boost::system::error_code, class UnderlyingCategory = boost::system::error_category>
	struct error_code
	{
		error_code() BOOST_NOEXCEPT
			: storage(nullptr)
		{
		}

		template <int Value, UnderlyingCategory const &(*Category)()>
		static error_code create()
		{
			static concrete_storage<Value, Category> const instance;
			return error_code(instance);
		}

		UnderlyingErrorCode to_underlying() const
		{
			if (storage)
			{
				return storage->to_underlying();
			}
			return UnderlyingErrorCode();
		}

		void clear() BOOST_NOEXCEPT
		{
			storage = nullptr;
		}

		int value() const BOOST_NOEXCEPT
		{
			return to_underlying().value();
		}

		UnderlyingCategory const &category() const BOOST_NOEXCEPT
		{
			return to_underlying().category();
		}

	private:

		struct basic_storage
		{
			virtual ~basic_storage()
			{
			}

			virtual UnderlyingErrorCode to_underlying() const = 0;
		};

		template <int Value, UnderlyingCategory const &(*Category)()>
		struct concrete_storage : basic_storage
		{
			virtual UnderlyingErrorCode to_underlying() const SILICIUM_OVERRIDE
			{
				return UnderlyingErrorCode(Value, Category());
			}
		};

		basic_storage const *storage;

		explicit error_code(basic_storage const &storage) BOOST_NOEXCEPT
			: storage(&storage)
		{
		}
	};

	BOOST_STATIC_ASSERT(sizeof(error_code<>) == sizeof(void *));

	template <class UnderlyingErrorCode, class UnderlyingCategory>
	bool operator == (error_code<UnderlyingErrorCode, UnderlyingCategory> const &left, error_code<UnderlyingErrorCode, UnderlyingCategory> const &right)
	{
		return left.to_underlying() == right.to_underlying();
	}

	template <class UnderlyingErrorCode, class UnderlyingCategory>
	inline SILICIUM_NORETURN void throw_error(error_code<UnderlyingErrorCode, UnderlyingCategory> error)
	{
		return throw_error(error.to_underlying());
	}

	template <class UnderlyingErrorCode, class UnderlyingCategory>
	std::size_t hash_value(error_code<UnderlyingErrorCode, UnderlyingCategory> const &value)
	{
		return std::hash<error_code<UnderlyingErrorCode, UnderlyingCategory>>()(value);
	}

	template <class UnderlyingErrorCode, class UnderlyingCategory>
	std::ostream &operator << (std::ostream &out, error_code<UnderlyingErrorCode, UnderlyingCategory> const &value)
	{
		return out << value.to_underlying();
	}
}

namespace std
{
	template <class UnderlyingErrorCode, class UnderlyingCategory>
	struct hash<Si::error_code<UnderlyingErrorCode, UnderlyingCategory>>
	{
		std::size_t operator()(Si::error_code<UnderlyingErrorCode, UnderlyingCategory> const &value) const
		{
			return boost::system::hash_value(value.to_underlying());
		}
	};
}

#endif
