
set( RTIO_INTERFACE_INCLUDE_DIR
     ${RTIO_SDK_ROOT_DIR}/libraries/standard/coreRTIO/source/interface )

set( ESP_TRANSPORT_SOURCES
     ${CMAKE_CURRENT_LIST_DIR}/transport/tls/network_esp.c )
set( ESP_TRANSPORT_INCLUDE_PUBLIC_DIRS
     ${CMAKE_CURRENT_LIST_DIR}/transport/tls/include )

set( ESP_OS_SOURCES
     ${CMAKE_CURRENT_LIST_DIR}/os/os_esp.c )
set( ESP_OS_INCLUDE_PUBLIC_DIRS
     ${CMAKE_CURRENT_LIST_DIR}/os/include )

