#ifndef SILICIUM_IDENTITY_HPP
#define SILICIUM_IDENTITY_HPP

namespace Si
{
    template <class T>
    struct identity
    {
        typedef T type;
    };
}

#endif
