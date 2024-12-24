
# Include RTIO library's source and header path variables.
include( ${CMAKE_SOURCE_DIR}/libraries/standard/coreRTIO/rtioFilePaths.cmake )

# Demo target, rtio_demo_copost_to_server.
set( DEMO_NAME "rtio_demo_copost_to_server" )
add_executable(
    ${DEMO_NAME}
        "${CMAKE_CURRENT_LIST_DIR}/rtio/${DEMO_NAME}/${DEMO_NAME}.c"
        ${RTIO_SOURCES}
)
target_link_libraries(
    ${DEMO_NAME}
    PRIVATE
        os_posix
        plaintext_posix
)
target_include_directories(
    ${DEMO_NAME}
    PUBLIC
        ${COMMON_TRANSPORT_PLAINTEXT_INCLUDE_PUBLIC_DIRS}
        ${RTIO_INCLUDE_PUBLIC_DIRS}
        ${RTIO_INCLUDE_INTERNEL_DIRS}
        "${CMAKE_CURRENT_LIST_DIR}/rtio/${DEMO_NAME}"
        ${LOGGING_INCLUDE_DIRS}
)

# Demo target, rtio_demo_simple.
set( DEMO_NAME "rtio_demo_simple" )
add_executable(
    ${DEMO_NAME}
        "${CMAKE_CURRENT_LIST_DIR}/rtio/${DEMO_NAME}/${DEMO_NAME}.c"
        ${RTIO_SOURCES}
)
target_link_libraries(
    ${DEMO_NAME}
    PRIVATE
        os_posix
        plaintext_posix
)
target_include_directories(
    ${DEMO_NAME}
    PUBLIC
        ${COMMON_TRANSPORT_PLAINTEXT_INCLUDE_PUBLIC_DIRS}
        ${RTIO_INCLUDE_PUBLIC_DIRS}
        ${RTIO_INCLUDE_INTERNEL_DIRS}
        "${CMAKE_CURRENT_LIST_DIR}/rtio/${DEMO_NAME}"
        ${LOGGING_INCLUDE_DIRS}
)

# Demo target, rtio_demo_obget.
set( DEMO_NAME "rtio_demo_obget" )
add_executable(
    ${DEMO_NAME}
        "${CMAKE_CURRENT_LIST_DIR}/rtio/${DEMO_NAME}/${DEMO_NAME}.c"
        ${RTIO_SOURCES}
)
target_link_libraries(
    ${DEMO_NAME}
    PRIVATE
        os_posix
        plaintext_posix
)
target_include_directories(
    ${DEMO_NAME}
    PUBLIC
        ${COMMON_TRANSPORT_PLAINTEXT_INCLUDE_PUBLIC_DIRS}
        ${RTIO_INCLUDE_PUBLIC_DIRS}
        ${RTIO_INCLUDE_INTERNEL_DIRS}
        "${CMAKE_CURRENT_LIST_DIR}/rtio/${DEMO_NAME}"
        ${LOGGING_INCLUDE_DIRS}
)

# Demo target, rtio_demo_tls.
set( DEMO_NAME "rtio_demo_tls" )
add_executable(
    ${DEMO_NAME}
        "${CMAKE_CURRENT_LIST_DIR}/rtio/${DEMO_NAME}/${DEMO_NAME}.c"
        ${RTIO_SOURCES}
)
target_link_libraries(
    ${DEMO_NAME}
    PRIVATE
        os_posix
        openssl_posix
)
target_include_directories(
    ${DEMO_NAME}
    PUBLIC
        ${COMMON_TRANSPORT_OPENSSL_INCLUDE_PUBLIC_DIRS}
        ${RTIO_INCLUDE_PUBLIC_DIRS}
        ${RTIO_INCLUDE_INTERNEL_DIRS}
        "${CMAKE_CURRENT_LIST_DIR}/rtio/${DEMO_NAME}"
        ${LOGGING_INCLUDE_DIRS}
)