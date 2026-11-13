#===============================================================================
# TMC9660 driver: build settings
#===============================================================================
# Read by the root CMakeLists.txt in both of its modes (plain CMake and the
# ESP-IDF component): target name, version, include directories, sources and
# ESP-IDF requirements. This file is the single source
# of the version; tests/CMakeLists.txt checks every other copy against it.
#===============================================================================

include_guard(GLOBAL)

set(TMC9660_TARGET_NAME "tmc9660")

#===============================================================================
# Version (single source of truth)
#===============================================================================
set(TMC9660_VERSION_MAJOR 0)
set(TMC9660_VERSION_MINOR 1)
set(TMC9660_VERSION_PATCH 0)
set(TMC9660_VERSION "${TMC9660_VERSION_MAJOR}.${TMC9660_VERSION_MINOR}.${TMC9660_VERSION_PATCH}")

#===============================================================================
# Generated version header (build tree, never the source tree)
#===============================================================================
set(TMC9660_VERSION_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/../inc/tmc9660_version.h.in")
set(TMC9660_VERSION_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/tmc9660_generated")
set(TMC9660_VERSION_HEADER "${TMC9660_VERSION_HEADER_DIR}/tmc9660_version.h")

# The directory must exist before ESP-IDF's idf_component_register() checks the include paths.
file(MAKE_DIRECTORY "${TMC9660_VERSION_HEADER_DIR}")
configure_file("${TMC9660_VERSION_TEMPLATE}" "${TMC9660_VERSION_HEADER}" @ONLY)

#===============================================================================
# Public include directories, sources, ESP-IDF requirements
#===============================================================================
set(TMC9660_PUBLIC_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../inc"
    "${TMC9660_VERSION_HEADER_DIR}"
)

# Header-only: the template implementation is src/tmc9660.ipp, included by inc/tmc9660.hpp.
set(TMC9660_SOURCE_FILES)

set(TMC9660_IDF_REQUIRES)
