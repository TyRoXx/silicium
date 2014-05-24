#include <silicium/process.hpp>
#include <silicium/build_result.hpp>
#include <silicium/directory_allocator.hpp>
#include <silicium/to_unique.hpp>
#include <silicium/git/repository.hpp>
#include <boost/asio.hpp>

namespace
{
	struct tcp_trigger
	{
		explicit tcp_trigger(boost::asio::io_service &io, boost::asio::ip::tcp::endpoint address)
			: m_acceptor(io, address, true)
		{
		}

		template <class Handler>
		void async_wait(Handler handler)
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

		void cancel()
		{
			m_acceptor.cancel();
		}

	private:

		boost::asio::ip::tcp::acceptor m_acceptor;
		std::unique_ptr<boost::asio::ip::tcp::socket> m_accepting;
	};

	void check_build(
			boost::filesystem::path const &source_location,
			boost::filesystem::path const &output_location)
	{
		auto const source = Si::git::open_repository(source_location);
		auto const master = Si::git::lookup(*source, "master");

	}
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		return 1;
	}
	boost::filesystem::path const source_location = argv[1];
	boost::filesystem::path const output_location = argv[2];

	auto const build = [&]
	{
		check_build(source_location, output_location);
	};

	boost::asio::io_service io;
	tcp_trigger external_build_trigger(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 12345));
	std::function<void ()> wait_for_trigger;
	wait_for_trigger = [&wait_for_trigger, &external_build_trigger, &build]
	{
		external_build_trigger.async_wait([&wait_for_trigger, &build]
		{
			build();
			wait_for_trigger();
		});
	};
	wait_for_trigger();
	io.run();
}
