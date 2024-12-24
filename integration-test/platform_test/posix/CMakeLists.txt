
project ("platform posix api simple test" )
cmake_minimum_required (VERSION 3.2.0)

include( "${RTIO_SDK_ROOT_DIR}/demos-posix/logging-stack/logging.cmake" )
include( "${RTIO_SDK_ROOT_DIR}/platform/posix/posixFilePaths.cmake" )

set( TEST_NAME "posix_api_simple_test" )

# CPP files are searched for supporting CI build checks that verify C++ linkage of the rtioHTTP library
file( GLOB TEST_FILE "${TEST_NAME}.c*" )

# Demo target.
add_executable(
    ${TEST_NAME}
        "${TEST_FILE}"
)

target_link_libraries(
    ${TEST_NAME}
    PRIVATE
        os_posix
        plaintext_posix
)

target_include_directories(
    ${TEST_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${LOGGING_INCLUDE_DIRS}
        ${COMMON_OS_INCLUDE_PUBLIC_DIRS}
)

