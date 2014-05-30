#include <silicium/asio/accepting_source.hpp>

namespace Si
{
	accepting_source::accepting_source(boost::asio::ip::tcp::acceptor &acceptor, boost::asio::yield_context &yield)
		: m_acceptor(acceptor)
		, m_yield(yield)
	{
	}

	boost::iterator_range<accepting_source::element_type const *> accepting_source::map_next(std::size_t)
	{
		return boost::iterator_range<element_type const *>();
	}

	accepting_source::element_type *accepting_source::copy_next(boost::iterator_range<element_type *> destination)
	{
		for (auto &client : destination)
		{
			client = std::make_shared<boost::asio::ip::tcp::socket>(m_acceptor.get_io_service());
			m_acceptor.async_accept(*client, m_yield);
		}
		return destination.end();
	}

	boost::uintmax_t accepting_source::minimum_size()
	{
		return 0;
	}

	boost::optional<boost::uintmax_t> accepting_source::maximum_size()
	{
		return boost::none;
	}

	std::size_t accepting_source::skip(std::size_t count)
	{
		for (size_t i = 0; i < count; ++i)
		{
			element_type thrown_away;
			if (copy_next(boost::make_iterator_range(&thrown_away, &thrown_away + 1)) == &thrown_away)
			{
				return i;
			}
		}
		return count;
	}
}
