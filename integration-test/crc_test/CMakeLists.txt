
project ("crc test")
cmake_minimum_required (VERSION 3.2.0)

set( TEST_NAME "crc_test" )

include( ${CMAKE_SOURCE_DIR}/libraries/standard/coreRTIO/rtioFilePaths.cmake )
add_definitions( -DRTIO_DO_NOT_USE_CUSTOM_CONFIG )

# CPP files are searched for supporting CI build checks that verify C++ linkage of the rtioHTTP library
file( GLOB TEST_FILE "${TEST_NAME}.c*" )

# TEST target.
add_executable(
    ${TEST_NAME}
        "${TEST_FILE}"
        "${RTIO_SOURCES}"
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
        ${COMMON_TRANSPORT_PLAINTEXT_INCLUDE_PUBLIC_DIRS}
        ${RTIO_INCLUDE_PUBLIC_DIRS}
        ${RTIO_INCLUDE_INTERNEL_DIRS}
        ${CMAKE_CURRENT_LIST_DIR}
        ${LOGGING_INCLUDE_DIRS}
)


file(COPY "${CMAKE_CURRENT_LIST_DIR}/crc_test_auto.sh"
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY "${CMAKE_CURRENT_LIST_DIR}/crc_check_data.py"
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
