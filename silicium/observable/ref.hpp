#ifndef SILICIUM_REACTIVE_REF_HPP
#define SILICIUM_REACTIVE_REF_HPP

#include <silicium/observable/ptr.hpp>

namespace Si
{
    template <class Observable>
    SILICIUM_USE_RESULT auto ref(Observable &identity)
#if !SILICIUM_COMPILER_HAS_AUTO_RETURN_TYPE
        -> ptr_observable<typename Observable::element_type, Observable *>
#endif
    {
        return ptr_observable<typename Observable::element_type, Observable *>(
            &identity);
    }
}

#endif
