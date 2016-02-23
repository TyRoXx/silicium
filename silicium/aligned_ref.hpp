#ifndef SILICIUM_ALIGNED_REF_HPP
#define SILICIUM_ALIGNED_REF_HPP

#include <boost/version.hpp>

#define SILICIUM_HAS_ALIGNED_REF (BOOST_VERSION >= 105600)

#if SILICIUM_HAS_ALIGNED_REF
#include <boost/align/is_aligned.hpp>
#include <silicium/optional.hpp>
#include <algorithm>

#if BOOST_VERSION >= 105900
#include <boost/align/assume_aligned.hpp>
#endif

namespace Si
{
	template <class Element, std::size_t Alignment>
	struct aligned_ref
	{
		static optional<aligned_ref> create(Element &data)
		{
			if (boost::alignment::is_aligned(Alignment, &data))
			{
				return aligned_ref(data);
			}
			return none;
		}

		Element &ref() const
		{
			return *m_data;
		}

	private:
		Element *m_data;

		explicit aligned_ref(Element &data)
		    : m_data(&data)
		{
		}
	};

	template <class From, class To, std::size_t Alignment>
	void copy(aligned_ref<From, Alignment> from, aligned_ref<To, Alignment> to,
	          std::size_t count)
	{
		// raw pointers to make it as easy as possible for the optimizer to
		// understand what is being done.
		void const *from_ptr = &from.ref();
		void *to_ptr = &to.ref();
#if BOOST_VERSION >= 105900
		BOOST_ALIGN_ASSUME_ALIGNED(from_ptr, Alignment);
		BOOST_ALIGN_ASSUME_ALIGNED(to_ptr, Alignment);
#endif
		std::copy(static_cast<From const *>(from_ptr),
		          static_cast<From const *>(from_ptr) + count,
		          static_cast<To *>(to_ptr));
	}
}
#endif
#endif
