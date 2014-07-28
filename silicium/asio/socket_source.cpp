#include <silicium/asio/socket_source.hpp>

namespace Si
{
	socket_source::socket_source(boost::asio::ip::tcp::socket &socket, boost::asio::yield_context &yield)
		: m_socket(&socket)
		, m_yield(&yield)
	{
	}

	boost::iterator_range<char const *> socket_source::map_next(std::size_t)
	{
		return boost::iterator_range<char const *>();
	}

	char *socket_source::copy_next(boost::iterator_range<char *> destination)
	{
		assert(m_socket);
		assert(m_yield);
		size_t const read = m_socket->async_read_some(boost::asio::buffer(destination.begin(), destination.size()), *m_yield);
		return destination.begin() + read;
	}

	boost::uintmax_t socket_source::minimum_size()
	{
		return 0;
	}

	boost::optional<boost::uintmax_t> socket_source::maximum_size()
	{
		return boost::none;
	}

	std::size_t socket_source::skip(std::size_t count)
	{
		assert(m_socket);
		assert(m_yield);
		std::size_t skipped = 0;
		while (skipped < count)
		{
			std::array<char, 1U << 12U> thrown_away;
			auto const rest = (count - skipped);
			assert(m_socket);
			assert(m_yield);
			auto const read = m_socket->async_read_some(boost::asio::buffer(thrown_away.data(), std::min(rest, thrown_away.size())), *m_yield);
			skipped += read;
			if (read == 0)
			{
				break;
			}
		}
		return skipped;
	}
}
