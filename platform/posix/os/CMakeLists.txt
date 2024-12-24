
set( RTIO_INTERFACE_INCLUDE_DIR
     ${MODULES_DIR}/standard/coreRTIO/source/interface )

add_library( os_posix
             "os_posix.c"
             "clock_posix.c" )

target_link_libraries( os_posix
                       pthread)

target_include_directories( os_posix
                            PUBLIC
                            ${RTIO_INTERFACE_INCLUDE_DIR}
                            ${COMMON_OS_INCLUDE_PUBLIC_DIRS}
                            ${LOGGING_INCLUDE_DIRS}  )
