#ifndef SILICIUM_PROGRAM_OPTIONS_HPP
#define SILICIUM_PROGRAM_OPTIONS_HPP

#include <silicium/config.hpp>

#define SILICIUM_HAS_PROGRAM_OPTIONS                                           \
    ((BOOST_VERSION >= 105800) || SILICIUM_HAS_RTTI)

#if SILICIUM_HAS_PROGRAM_OPTIONS
#include <boost/program_options.hpp>
#endif

#endif
