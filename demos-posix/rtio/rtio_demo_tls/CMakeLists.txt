# Project information.
cmake_minimum_required( VERSION 3.2.0 )
project( RTIO-SDK-C-Demos
         VERSION 0.1.0
         LANGUAGES C CXX )
         
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

# Get root dirs and the cmake-files it depends on.
include( "../../demosRoot.cmake" )
include( "${RTIO_SDK_ROOT_DIR}/tools/cmake/rtioSDK_POSIX_OpenSSL.cmake" )
include( "${PLATFORM_DEMOS_ROOT_DIR}/logging-stack/logging.cmake" )

message( "---------- RTIO SDK Sources and Include directories -----------" )
message( "RTIO_SDK_SOURCES:" )
foreach( item IN LISTS RTIO_SDK_SOURCES )
    message( STATUS "${item}" )
endforeach()
message( "RTIO_SDK_INCLUDE_DIRS:" )
foreach( item IN LISTS RTIO_SDK_INCLUDE_DIRS )
    message( STATUS "${item}" )
endforeach()
message( "---------------------------------------------------------------" )


set( DEMO_NAME "rtio_demo_tls" )

# Demo target.
add_executable(
    ${DEMO_NAME}
        "rtio_demo_tls.c"
        ${RTIO_SDK_SOURCES}
)

target_link_libraries(
    ${DEMO_NAME}
    PRIVATE
        pthread
        ssl
        crypto
)

target_include_directories(
    ${DEMO_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${LOGGING_INCLUDE_DIRS}
        ${RTIO_SDK_INCLUDE_DIRS}
)

# Copy certificates to the build directory.
set(CERT_DOWNLOAD_DIR ${PLATFORM_DEMOS_ROOT_DIR}/certificates)
file(COPY "${CERT_DOWNLOAD_DIR}"
     DESTINATION ${CMAKE_BINARY_DIR})