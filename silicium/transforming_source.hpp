#ifndef SILICIUM_TRANSFORMING_SOURCE_HPP
#define SILICIUM_TRANSFORMING_SOURCE_HPP

#include <silicium/source.hpp>

namespace Si
{
	template <class To, class From, class Transformation>
	struct transforming_source SILICIUM_FINAL : source<To>
	{
		template <class Transformation2>
		explicit transforming_source(source<From> &original, Transformation2 &&transform)
			: original(&original)
			, transform(std::forward<Transformation2>(transform))
		{
		}

		virtual boost::iterator_range<To const *> map_next(std::size_t size) override
		{
			(void)size;
			return {};
		}

		virtual To *copy_next(boost::iterator_range<To *> destination) override
		{
			auto i = boost::begin(destination);
			for (; i != boost::end(destination); ++i)
			{
				assert(original);
				auto next = Si::get(*original);
				if (!next)
				{
					break;
				}
				*i = transform(std::move(*next));
			}
			return i;
		}

		virtual boost::uintmax_t minimum_size() override
		{
			return original->minimum_size();
		}

		virtual boost::optional<boost::uintmax_t> maximum_size() override
		{
			return original->maximum_size();
		}

		virtual std::size_t skip(std::size_t count) override
		{
			return original->skip(count);
		}

	private:

		source<From> *original = nullptr;
		Transformation transform;
	};

	template <class To, class From, class Transformation>
	transforming_source<To, From, typename std::decay<Transformation>::type>
	make_transforming_source(source<From> &original, Transformation &&transform)
	{
		return transforming_source<To, From, typename std::decay<Transformation>::type>(original, std::forward<Transformation>(transform));
	}
}

#endif
