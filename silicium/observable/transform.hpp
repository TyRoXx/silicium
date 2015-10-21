#ifndef SILICIUM_REACTIVE_TRANSFORM_HPP
#define SILICIUM_REACTIVE_TRANSFORM_HPP

#include <silicium/observable/observer.hpp>
#include <silicium/config.hpp>
#include <silicium/detail/proper_value_function.hpp>
#include <type_traits>
#include <utility>
#include <cassert>
#include <functional>
#include <boost/config.hpp>

#define SILICIUM_HAS_TRANSFORM_OBSERVABLE (SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES && !SILICIUM_GCC46)

namespace Si
{
#if SILICIUM_HAS_TRANSFORM_OBSERVABLE
	template <class Transform, class Original>
	struct transformation : private observer<typename Original::element_type>
	{
		typedef typename std::result_of<Transform(typename Original::element_type)>::type element_type;

		BOOST_STATIC_ASSERT(!std::is_void<element_type>::value);

		typedef typename Original::element_type from_type;

		transformation()
		    : receiver(nullptr)
		{
		}

		template <class Transform2>
		explicit transformation(Transform2 &&transform, Original original)
		    : transform(std::forward<Transform2>(transform))
		    , original(std::move(original))
		    , receiver(nullptr)
		{
		}

#if !SILICIUM_COMPILER_GENERATES_MOVES
		transformation(transformation &&other)
		    : transform(std::move(other.transform))
		    , original(std::move(other.original))
		    , receiver(std::move(other.receiver))
		{
		}

		transformation &operator=(transformation &&other)
		{
			// TODO: exception safety
			transform = std::move(other.transform);
			original = std::move(other.original);
			receiver = std::move(other.receiver);
			return *this;
		}
#else
		transformation(transformation &&other) = default;
		transformation &operator=(transformation &&other) = default;
#endif

		template <class Observer>
		void async_get_one(Observer &&receiver)
		{
			assert(!this->receiver);
			this->receiver = receiver.get();
			original.async_get_one(
			    extend(std::forward<Observer>(receiver),
			           observe_by_ref(static_cast<observer<typename Original::element_type> &>(*this))));
		}

		Original &get_input()
		{
			return original;
		}

	private:
#if SILICIUM_DETAIL_HAS_PROPER_VALUE_FUNCTION
		typedef typename detail::proper_value_function<Transform, element_type, from_type>::type proper_transform;
		proper_transform transform; // TODO empty base optimization
#else
		Transform transform;
#endif

		Original original;
		observer<element_type> *receiver;

		virtual void got_element(from_type value) SILICIUM_OVERRIDE
		{
			assert(receiver);
			auto *receiver_copy = receiver;
			receiver = nullptr;
			receiver_copy->got_element(transform(std::move(value)));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(receiver);
			auto *receiver_copy = receiver;
			receiver = nullptr;
			receiver_copy->ended();
		}

		SILICIUM_DELETED_FUNCTION(transformation(transformation const &))
		SILICIUM_DELETED_FUNCTION(transformation &operator=(transformation const &))
	};

	namespace detail
	{
		template <class Argument, class F>
		F &&nothingify(F &&f, std::false_type)
		{
			return std::forward<F>(f);
		}

		template <class F, class Argument>
		struct void_to_nothing_wrapper
		{
#if defined(_MSC_VER) && (_MSC_VER < 1900)
			typename detail::proper_value_function<F, void, Argument>::type original;
#else
			F original;
#endif

			template <class... Args>
			nothing operator()(Args &&... args) const
			{
				original(std::forward<Args>(args)...);
				return {};
			}
		};

		template <class Argument, class F>
		void_to_nothing_wrapper<typename std::decay<F>::type, Argument> nothingify(F &&f, std::true_type)
		{
			return {std::forward<F>(f)};
		}
	}

	template <class Transform, class Original>
	auto transform(Original &&original, Transform &&transform)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
	    -> transformation<
	        typename std::decay<decltype(detail::nothingify<typename std::decay<Original>::type::element_type>(
	            std::forward<Transform>(transform),
	            std::is_void<
	                decltype(transform(std::declval<typename std::decay<Original>::type::element_type>()))>()))>::type,
	        typename std::decay<Original>::type>
#endif
	{
		typedef typename std::decay<Original>::type clean_original;
		typedef std::is_void<decltype(transform(std::declval<typename clean_original::element_type>()))> returns_void;
		auto transform_that_returns_non_void = detail::nothingify<typename clean_original::element_type>(
		    std::forward<Transform>(transform), returns_void());
		return transformation<typename std::decay<decltype(transform_that_returns_non_void)>::type, clean_original>(
		    std::move(transform_that_returns_non_void), std::forward<Original>(original));
	}
#endif
}

#endif
