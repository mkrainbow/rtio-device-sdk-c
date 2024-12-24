# Get root dirs and the cmake-files it depends on.
include( "../../../demosRoot.cmake" )
include( "${RTIO_SDK_ROOT_DIR}/tools/cmake/rtioSDK_ESP32.cmake" )
include( "${PLATFORM_DEMOS_ROOT_DIR}/logging-stack/logging.cmake" )


# Demo Sources.
idf_component_register(

    SRCS
        "wifi_init.c"
         "rtio_demo_tls.c"
        ${RTIO_SDK_SOURCES}
        
    INCLUDE_DIRS 
        ${CMAKE_CURRENT_LIST_DIR}
        ${LOGGING_INCLUDE_DIRS}
        ${RTIO_SDK_INCLUDE_DIRS}
)

