# Project information.
message("--------------------------start--------------------------------")
cmake_minimum_required( VERSION 3.2.0 )
project( RTIO-SDK-C
         VERSION 1.0.0
         LANGUAGES C CXX )
         
# Allow the project to be organized into folders.
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

# Use C90 if not specified.
if( NOT DEFINED CMAKE_C_STANDARD )
    set( CMAKE_C_STANDARD 90 )
endif()
if( NOT DEFINED CMAKE_C_STANDARD_REQUIRED )
    set( CMAKE_C_STANDARD_REQUIRED ON )
endif()

# Do not allow in-source build.
if( ${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR} )
    message( FATAL_ERROR "In-source build is not allowed. Please build in a separate directory, such as ${PROJECT_SOURCE_DIR}/build." )
endif()

# Set the platform named based on the host OS if not defined.
if( NOT DEFINED PLATFORM_NAME )
    if( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
        set( PLATFORM_NAME "posix" CACHE STRING "Port to use for building the SDK." )
    else()
        message( FATAL_ERROR "${CMAKE_SYSTEM_NAME} is not a supported platform." )
    endif()
else()
    set( PLATFORM_NAME ${PLATFORM_NAME} CACHE STRING "Port to use for building the SDK." )
endif()
MESSAGE( STATUS "PLATFORM_NAME: " ${PLATFORM_NAME} )

# Set global path variables.
include( "root.cmake" )
set(DEMOS_DIR "${RTIO_SDK_ROOT_DIR}/demos-posix" CACHE INTERNAL "C SDK demos root.")
set(PLATFORM_DIR "${RTIO_SDK_ROOT_DIR}/platform" CACHE INTERNAL "C SDK platform root.")
set(MODULES_DIR "${RTIO_SDK_ROOT_DIR}/libraries" CACHE INTERNAL "C SDK modules root.")
set(3RDPARTY_DIR "${MODULES_DIR}/3rdparty" CACHE INTERNAL "3rdparty libraries root.")
include( "${DEMOS_DIR}/logging-stack/logging.cmake" )

# Configure options to always show in CMake GUI.
option( BUILD_TESTS
        "Set this to ON to build test executables."
        OFF )
option( BUILD_DEMOS
        "Set this to ON to build demo executables."
        ON )

# Unity test framework does not export the correct symbols for DLLs.
set( ALLOW_SHARED_LIBRARIES ON )

include( CMakeDependentOption )
CMAKE_DEPENDENT_OPTION( BUILD_SHARED_LIBS
                        "Set this to ON to build all libraries as shared libraries. When OFF, libraries build as static libraries."
                        ON "${ALLOW_SHARED_LIBRARIES}"
                        OFF )

# Set output directories.
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )


# Find thread library.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)

# Add platform.
add_subdirectory( platform )

if(BUILD_DEMOS)
    # Add build configuration for demos.
    include( "${DEMOS_DIR}/rtioAllDemos.cmake" )
endif()
if(BUILD_TESTS)
    # Add build configuration for integration tests.
    add_subdirectory( integration-test )
endif()

# Copy certificates to the build directory.
set(CERT_DOWNLOAD_DIR ${DEMOS_DIR}/certificates)
file(COPY "${CERT_DOWNLOAD_DIR}"
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

