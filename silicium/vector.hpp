#ifndef SILICIUM_VECTOR_HPP
#define SILICIUM_VECTOR_HPP

#include <silicium/config.hpp>
#include <boost/container/vector.hpp>

namespace Si
{
	template <class Element, class Allocator = std::allocator<Element>>
	struct vector
	{
		typedef Element element_type;
		typedef std::size_t size_type;
		typedef typename boost::container::vector<Element, Allocator>::iterator iterator;
		typedef typename boost::container::vector<Element, Allocator>::const_iterator const_iterator;

		vector() BOOST_NOEXCEPT
		{
		}

		vector(vector &&other) BOOST_NOEXCEPT
			: m_impl(std::move(other.m_impl))
		{
		}

		vector &operator = (vector &&other) BOOST_NOEXCEPT
		{
			m_impl = std::move(other.m_impl);
			return *this;
		}

#if !SILICIUM_VC2012
		vector(std::initializer_list<Element> elements)
			//Boost 1.56 is missing the initializer_list ctor
			: m_impl(elements.begin(), elements.end())
		{
		}
#endif

		SILICIUM_DISABLE_COPY(vector)
	public:

		vector copy() const
		{
			return vector{m_impl};
		}

		bool empty() const BOOST_NOEXCEPT
		{
			return m_impl.empty();
		}

		size_type size() const BOOST_NOEXCEPT
		{
			return m_impl.size();
		}

		element_type *data() BOOST_NOEXCEPT
		{
			return m_impl.data();
		}

		element_type const *data() const BOOST_NOEXCEPT
		{
			return m_impl.data();
		}

		iterator begin() BOOST_NOEXCEPT
		{
			return m_impl.begin();
		}

		const_iterator begin() const BOOST_NOEXCEPT
		{
			return m_impl.begin();
		}

		iterator end() BOOST_NOEXCEPT
		{
			return m_impl.end();
		}

		const_iterator end() const BOOST_NOEXCEPT
		{
			return m_impl.end();
		}

#if SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES
		template <class ...Args>
		void emplace_back(Args &&...args)
		{
			m_impl.emplace_back(std::forward<Args>(args)...);
		}
#endif

	private:

		boost::container::vector<Element, Allocator> m_impl;

		explicit vector(boost::container::vector<Element, Allocator> impl)
			: m_impl(std::move(impl))
		{
		}
	};

	template <class Element, class Allocator>
	bool operator == (vector<Element, Allocator> const &left, vector<Element, Allocator> const &right)
	{
		return (left.size() == right.size()) && std::equal(left.begin(), left.end(), right.begin());
	}
}

#endif
