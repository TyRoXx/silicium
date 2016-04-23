#ifndef SILICIUM_CONNECTING_OBSERVABLE_HPP
#define SILICIUM_CONNECTING_OBSERVABLE_HPP

#include <silicium/exchange.hpp>
#include <silicium/observable/observer.hpp>
#include <algorithm>
#include <boost/asio/ip/tcp.hpp>

namespace Si
{
    namespace asio
    {
        struct connecting_observable
        {
            typedef boost::system::error_code element_type;

            explicit connecting_observable(
                boost::asio::ip::tcp::socket &socket,
                boost::asio::ip::tcp::endpoint destination)
                : socket(&socket)
                , destination(destination)
                , receiver_(nullptr)
            {
            }

            void async_get_one(ptr_observer<observer<element_type>> receiver)
            {
                assert(socket);
                assert(!receiver_);
                receiver_ = receiver.get();
                socket->async_connect(
                    destination, [this](boost::system::error_code ec)
                    {
                        Si::exchange(receiver_, nullptr)->got_element(ec);
                    });
            }

        private:
            boost::asio::ip::tcp::socket *socket;
            boost::asio::ip::tcp::endpoint destination;
            observer<element_type> *receiver_;
        };
    }
}

#endif
