#include <silicium/terminate_on_exception.hpp>
#include <silicium/config.hpp>
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <memory>
#include <array>
#include <iostream>

#if SILICIUM_HAS_EXCEPTIONS
#include <thread>

int main()
{
    boost::asio::io_service io;
    boost::asio::ip::tcp::acceptor acceptor(
        io,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4(), 8080));
    boost::uintmax_t visitor_count = 100000000;
    for (;;)
    {
        auto client = std::make_shared<boost::asio::ip::tcp::socket>(io);
        acceptor.accept(*client);
        auto visitor_id = ++visitor_count;
        std::thread(
            [client, visitor_id]
            {
                {
                    std::array<char, 1024> receive_buffer;
                    boost::system::error_code ec;
                    client->read_some(boost::asio::buffer(receive_buffer), ec);
                    if (!!ec)
                    {
                        return;
                    }
                }

                {
                    const auto response =
                        boost::str(boost::format("HTTP/1.0 200 OK\r\n"
                                                 "Content-Length: 9\r\n"
                                                 "Connection: close\r\n"
                                                 "\r\n"
                                                 "%1%") %
                                   visitor_id);
                    boost::system::error_code ec;
                    auto sent =
                        client->send(boost::asio::buffer(response), 0, ec);
                    if (ec || (sent != response.size()))
                    {
                        std::cerr << "Could not send\n";
                        return;
                    }
                }

                client->shutdown(boost::asio::ip::tcp::socket::shutdown_both);

                for (;;)
                {
                    std::array<char, 1024> receive_buffer;
                    boost::system::error_code ec;
                    client->receive(boost::asio::buffer(receive_buffer), 0, ec);
                    if (!!ec)
                    {
                        break;
                    }
                }
            })
            .detach();
    }
}
#else
int main()
{
    std::cerr << "This example requires exception support\n";
}
#endif
