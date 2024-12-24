include_guard(GLOBAL) 
get_filename_component(__root_dir "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
set(RTIO_SDK_ROOT_DIR ${__root_dir} CACHE INTERNAL "C SDK source root.")
message( STATUS "RTIO_SDK_ROOT_DIR: " ${RTIO_SDK_ROOT_DIR} )