# # Include filepaths for source and include.
# include( ${PLATFORM_DIR}/posix/posixFilePaths.cmake )

# Create target for sockets utility.
add_library( sockets_posix
                ${SOCKETS_SOURCES} )

target_include_directories( sockets_posix
                            PUBLIC
                                ${TRANSPORT_INTERNEL_INCLUDE_PUBLIC_DIRS}
                                ${LOGGING_INCLUDE_DIRS}
                                ${RTIO_INTERFACE_INCLUDE_DIR} )

# Create target for plaintext transport.
add_library( plaintext_posix
             ${PLAINTEXT_TRANSPORT_SOURCES} )

target_include_directories( plaintext_posix
             PUBLIC
                 ${COMMON_TRANSPORT_PLAINTEXT_INCLUDE_PUBLIC_DIRS}
                 ${TRANSPORT_INTERNEL_INCLUDE_PUBLIC_DIRS}
                 ${LOGGING_INCLUDE_DIRS}
                 ${RTIO_INTERFACE_INCLUDE_DIR} )
             
target_link_libraries( plaintext_posix
                       PUBLIC
                           sockets_posix )

# Create target for POSIX implementation of OpenSSL.
add_library( openssl_posix
                ${OPENSSL_TRANSPORT_SOURCES} )

target_include_directories( openssl_posix
                PUBLIC
		            # /usr/local/ssl/include
                    ${COMMON_TRANSPORT_OPENSSL_INCLUDE_PUBLIC_DIRS}
                    ${TRANSPORT_INTERNEL_INCLUDE_PUBLIC_DIRS}
                    ${LOGGING_INCLUDE_DIRS}
                    ${RTIO_INTERFACE_INCLUDE_DIR} )
                    
target_link_libraries( openssl_posix
                       PUBLIC
                          sockets_posix
                          # This variable is set by the built-in FindOpenSSL.cmake
                          # and contains the path to the actual library.
                          ${OPENSSL_LIBRARIES}
			            #   /usr/local/ssl/lib
                       PRIVATE
                          # SSL uses Threads and on some platforms require
                          # explicit linking.
                          Threads::Threads
                          # SSL uses Dynamic Loading and on some platforms
                          # requires explicit linking.
                          ${CMAKE_DL_LIBS} )
