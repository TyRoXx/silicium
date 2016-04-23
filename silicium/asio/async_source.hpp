#ifndef SILICIUM_ASIO_ASYNC_SOURCE_HPP
#define SILICIUM_ASIO_ASYNC_SOURCE_HPP

#include <silicium/asio/async.hpp>
#include <silicium/variant.hpp>
#include <silicium/iterator_range.hpp>

#define SILICIUM_HAS_ASIO_ASYNC_SOURCE SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE

namespace Si
{
    namespace asio
    {
#if SILICIUM_HAS_ASIO_ASYNC_SOURCE
        template <class BackgroundDispatcher, class Source>
        struct async_source
        {
            typedef
                typename std::decay<Source>::type::element_type element_type;
            typedef Si::variant<iterator_range<element_type const *>,
                                element_type *> result_type;

            async_source(BackgroundDispatcher background, Source source)
                : m_background(std::forward<BackgroundDispatcher>(background))
                , m_source(std::forward<Source>(source))
            {
            }

            template <class HandlerDispatcher, class Handler>
            auto async_get(iterator_range<element_type *> buffer,
                           HandlerDispatcher &dispatcher,
                           Handler &&handle_result)
            {
                return Si::asio::async(
                    dispatcher, m_background,
                    [this, buffer]() -> result_type
                    {
                        {
                            iterator_range<element_type const *> const mapped =
                                m_source.map_next(
                                    (std::numeric_limits<std::size_t>::max)());
                            if (!mapped.empty())
                            {
                                return mapped;
                            }
                        }
                        return m_source.copy_next(buffer);
                    },
                    [SILICIUM_CAPTURE_EXPRESSION(
                        handle_result, std::forward<Handler>(handle_result))](
                        result_type result) mutable
                    {
                        std::forward<Handler>(handle_result)(result);
                    });
            }

        private:
            BackgroundDispatcher m_background;
            Source m_source;
        };
#endif
    }
}

#endif
