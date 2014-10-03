#ifndef SILICIUM_TCP_TRIGGER_HPP
#define SILICIUM_TCP_TRIGGER_HPP

#include <boost/asio.hpp>
#include <memory>

namespace Si
{
	struct tcp_trigger
	{
		explicit tcp_trigger(boost::asio::io_service &io, boost::asio::ip::tcp::endpoint address);
		template <class Handler>
		void async_wait(Handler handler);

	private:

		boost::asio::ip::tcp::acceptor m_acceptor;
		std::unique_ptr<boost::asio::ip::tcp::socket> m_accepting;
	};

	template <class Handler>
	void tcp_trigger::async_wait(Handler handler)
	{
		if (m_accepting)
		{
			m_accepting->close();
		}
		m_accepting.reset(new boost::asio::ip::tcp::socket(m_acceptor.get_io_service()));
		m_acceptor.async_accept(*m_accepting, [this, handler](boost::system::error_code error)
		{
			if (error)
			{
				//TODO
			}
			else
			{
				m_accepting->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
				handler();
			}
		});
	}
}

#endif
