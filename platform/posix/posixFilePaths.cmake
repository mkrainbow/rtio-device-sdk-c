
set( RTIO_INTERFACE_INCLUDE_DIR
     ${RTIO_SDK_ROOT_DIR}/libraries/standard/coreRTIO/source/interface )

# Sockets utility source files.
set( SOCKETS_SOURCES
     ${CMAKE_CURRENT_LIST_DIR}/transport/internel/sockets_posix.c )

# Plaintext transport source files.
set( PLAINTEXT_TRANSPORT_SOURCES
     ${CMAKE_CURRENT_LIST_DIR}/transport/plaintext/plaintext_posix.c )

# OpenSSL transport source files.
set( OPENSSL_TRANSPORT_SOURCES
     ${CMAKE_CURRENT_LIST_DIR}/transport/openssl/openssl_posix.c )


# Transport Public Include directories.
set( COMMON_TRANSPORT_PLAINTEXT_INCLUDE_PUBLIC_DIRS
     ${CMAKE_CURRENT_LIST_DIR}/transport/plaintext/include )
set( COMMON_TRANSPORT_OPENSSL_INCLUDE_PUBLIC_DIRS
     ${CMAKE_CURRENT_LIST_DIR}/transport/openssl/include )

# Transport Private Include directories.
set( TRANSPORT_INTERNEL_INCLUDE_PUBLIC_DIRS
     ${CMAKE_CURRENT_LIST_DIR}/transport/internel/include )


# OS Public Include directories.
set( COMMON_OS_INCLUDE_PUBLIC_DIRS
     ${RTIO_SDK_ROOT_DIR}/platform/posix/os/include )

set( COMMON_OS_SOURCES
     ${CMAKE_CURRENT_LIST_DIR}/os/os_posix.c
     ${CMAKE_CURRENT_LIST_DIR}/os/clock_posix.c)
