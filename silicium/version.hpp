#ifndef SILICIUM_VERSION_HPP
#define SILICIUM_VERSION_HPP

#define SILICIUM_MAJOR_VERSION 0
#define SILICIUM_MINOR_VERSION 9
#define SILICIUM_PATCH_VERSION 0

#define SILICIUM_MAKE_VERSION(major, minor, patch)                             \
    ((((major * 100) + minor) * 100) + patch)
#define SILICIUM_VERSION                                                       \
    SILICIUM_MAKE_VERSION(SILICIUM_MAJOR_VERSION, SILICIUM_MINOR_VERSION,      \
                          SILICIUM_PATCH_VERSION)

#endif
