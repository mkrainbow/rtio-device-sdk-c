
# Set the platform named based on the host OS if not defined.
if( NOT DEFINED PLATFORM_NAME )
    if( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
        set( PLATFORM_NAME "posix" CACHE STRING "Port to use for building the SDK." )
    else()
        message( FATAL_ERROR "${CMAKE_SYSTEM_NAME} is not a supported platform." )
    endif()
else()
    set( PLATFORM_NAME ${PLATFORM_NAME} CACHE STRING "Port to use for building the SDK." )
endif()

MESSAGE( STATUS "PLATFORM_NAME: " ${PLATFORM_NAME} )

# Add each subdirectory in the current directory with a CMakeLists.txt file in it
file(GLOB_RECURSE test_modules "${CMAKE_CURRENT_LIST_DIR}/*/CMakeLists.txt")
foreach(module IN LISTS test_modules)
    get_filename_component(DIR_PATH "${module}" DIRECTORY)
    MESSAGE( STATUS "Adding test: " ${module} )
    add_subdirectory(${DIR_PATH})
endforeach()
