# Include filepaths for source and include.
include(posixFilePaths.cmake)

# Add the transport targets
add_subdirectory( ${CMAKE_CURRENT_LIST_DIR}/os )
add_subdirectory( ${CMAKE_CURRENT_LIST_DIR}/transport )
