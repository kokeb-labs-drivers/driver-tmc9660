#===============================================================================
# TMC9660 driver: root CMake
#===============================================================================
# One file, two build systems:
#
# - ESP-IDF (ESP_PLATFORM): this folder is an ESP-IDF component. Point
#   EXTRA_COMPONENT_DIRS at it, check it out under components/, or add it as a
#   component-manager git dependency. menuconfig reads _config/Kconfig.
# - Any other CMake build (host, STM32, RP2040, nRF, ...): one INTERFACE target,
#   tmc9660, consumed as tmc9660::tmc9660 both through add_subdirectory() and
#   through find_package(tmc9660) after install.
#
#   cmake -S . -B build -DTMC9660_BUILD_TESTS=ON
#   cmake --build build && ctest --test-dir build --output-on-failure
#===============================================================================
cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/tmc9660_build_settings.cmake)

#===============================================================================
# ESP-IDF component
#===============================================================================
if(ESP_PLATFORM)
    idf_component_register(
        SRCS ${TMC9660_SOURCE_FILES}
        INCLUDE_DIRS ${TMC9660_PUBLIC_INCLUDE_DIRS}
        REQUIRES ${TMC9660_IDF_REQUIRES}
        KCONFIG ${CMAKE_CURRENT_LIST_DIR}/_config/Kconfig
    )
    if(TMC9660_SOURCE_FILES)
        target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_20)
    else()
        target_compile_features(${COMPONENT_LIB} INTERFACE cxx_std_20)
    endif()
    return()
endif()

project(tmc9660_driver
    VERSION ${TMC9660_VERSION}
    DESCRIPTION "C++20 driver for the TMC9660 servo controller and gate driver (bootloader, parameter mode and register mode over SPI or UART)"
    LANGUAGES CXX)

include(GNUInstallDirs)

# ISO C++20 for every target this project builds (tests, fuzzers, examples), never the GNU dialect
# that CMake picks by default (handbook KB-CPP-001, KB-CPP20-001). CXX_EXTENSIONS does not travel
# through an INTERFACE target, so each folder that builds a target sets it as well.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# The installed layout keeps the source tree's inc/ + src/ pair, because tmc9660.hpp includes
# "../src/tmc9660.ipp" relative to itself:
#   <prefix>/include/tmc9660/inc/*.hpp, tmc9660_version.h
#   <prefix>/include/tmc9660/src/tmc9660.ipp
set(TMC9660_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/tmc9660/inc")

#===============================================================================
# Target
#===============================================================================
add_library(${TMC9660_TARGET_NAME} INTERFACE)
target_include_directories(${TMC9660_TARGET_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
        $<BUILD_INTERFACE:${TMC9660_VERSION_HEADER_DIR}>
        $<INSTALL_INTERFACE:${TMC9660_INSTALL_INCLUDEDIR}>
)
target_compile_features(${TMC9660_TARGET_NAME} INTERFACE cxx_std_20)

# tmc9660::tmc9660 everywhere: the alias for add_subdirectory(), EXPORT_NAME for the installed package.
add_library(tmc9660::tmc9660 ALIAS ${TMC9660_TARGET_NAME})
set_target_properties(${TMC9660_TARGET_NAME} PROPERTIES EXPORT_NAME tmc9660)

option(TMC9660_ENABLE_WARNINGS "Add -Wall -Wextra -Wpedantic to every target that links tmc9660::tmc9660" OFF)
if(TMC9660_ENABLE_WARNINGS)
    target_compile_options(${TMC9660_TARGET_NAME} INTERFACE -Wall -Wextra -Wpedantic)
endif()

#===============================================================================
# Host tests, documentation snippets, install check (tests/)
#===============================================================================
option(TMC9660_BUILD_TESTS "Build the host tests in tests/" OFF)
option(TMC9660_BUILD_FUZZERS "Also build the libFuzzer targets in tests/fuzz/ (Clang only)" OFF)
if(TMC9660_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

#===============================================================================
# Install and package export
#===============================================================================
include(CMakePackageConfigHelpers)

install(TARGETS ${TMC9660_TARGET_NAME} EXPORT tmc9660Targets
    INCLUDES DESTINATION ${TMC9660_INSTALL_INCLUDEDIR})

install(DIRECTORY inc/ DESTINATION ${TMC9660_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
install(FILES "${TMC9660_VERSION_HEADER}" DESTINATION ${TMC9660_INSTALL_INCLUDEDIR})
install(DIRECTORY src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tmc9660/src
    FILES_MATCHING PATTERN "*.ipp")

install(EXPORT tmc9660Targets
    FILE tmc9660Targets.cmake
    NAMESPACE tmc9660::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tmc9660)

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/tmc9660Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/tmc9660Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tmc9660)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/tmc9660ConfigVersion.cmake"
    VERSION ${TMC9660_VERSION}
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/tmc9660Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/tmc9660ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tmc9660)
