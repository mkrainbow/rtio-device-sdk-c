include( "${CMAKE_CURRENT_LIST_DIR}/../root.cmake" )

get_filename_component(__root_dir "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
set(PLATFORM_DEMOS_ROOT_DIR ${__root_dir} CACHE INTERNAL "C SDK source root.")
message( STATUS "PLATFORM_DEMOS_ROOT_DIR: " ${__root_dir} )