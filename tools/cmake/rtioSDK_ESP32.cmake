include( ${CMAKE_CURRENT_LIST_DIR}/../../root.cmake )

include( ${RTIO_SDK_ROOT_DIR}/libraries/standard/coreRTIO/rtioFilePaths.cmake )
include( ${RTIO_SDK_ROOT_DIR}/platform/esp32/esp32FilePaths.cmake )

set( RTIO_SDK_SOURCES
     ${RTIO_SOURCES}
     ${ESP_TRANSPORT_SOURCES}
     ${ESP_OS_SOURCES} )

set( RTIO_SDK_INCLUDE_DIRS
     ${RTIO_INTERFACE_INCLUDE_DIR}
     ${ESP_TRANSPORT_INCLUDE_PUBLIC_DIRS}
     ${ESP_OS_INCLUDE_PUBLIC_DIRS}
     ${RTIO_INCLUDE_INTERNEL_DIRS}
     ${RTIO_INCLUDE_PUBLIC_DIRS} )
