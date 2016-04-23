#ifndef SILICIUM_BYTE_ORDER_INTRINSICS_HPP
#define SILICIUM_BYTE_ORDER_INTRINSICS_HPP

#include <silicium/config.hpp>

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace Si
{
    inline boost::uint32_t ntoh32(boost::uint32_t value)
    {
        return ntohl(value);
    }

    inline boost::uint32_t hton32(boost::uint32_t value)
    {
        return htonl(value);
    }

    inline boost::uint16_t hton16(boost::uint16_t value)
    {
        return htons(value);
    }

    inline boost::uint16_t ntoh16(boost::uint16_t value)
    {
        return ntohs(value);
    }
}

#ifdef _MSC_VER
#include <stdlib.h>
namespace Si
{
    inline boost::uint16_t bswap16(boost::uint16_t value)
    {
        return ::_byteswap_ushort(value);
    }

    inline boost::uint32_t bswap32(boost::uint32_t value)
    {
        return ::_byteswap_ulong(value);
    }

    inline boost::uint64_t bswap64(boost::uint64_t value)
    {
        return ::_byteswap_uint64(value);
    }
}
#else
#ifndef __clang__
#include <byteswap.h>
#endif
namespace Si
{
    inline boost::uint16_t bswap16(boost::uint16_t value)
    {
        // provided for symmetry with Visual C++ bswap16
        return static_cast<boost::uint16_t>((value >> 8) | (value << 8));
    }

    inline boost::uint32_t bswap32(boost::uint32_t value)
    {
        return __builtin_bswap32(value);
    }

    inline boost::uint64_t bswap64(boost::uint64_t value)
    {
        return __builtin_bswap64(value);
    }
}
#endif

#endif
