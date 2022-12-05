# Included by platform-specific triplets to configure third party dependencies.
# Do not use as a triplet directly.

# Default to static linkage for all dependencies.
set(VCPKG_LIBRARY_LINKAGE static)

# Override linkage to dynamic only for these.
if(PORT MATCHES "openal-soft|qt5")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()
