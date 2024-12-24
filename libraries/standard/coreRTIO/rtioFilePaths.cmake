
include( ${RTIO_SDK_ROOT_DIR}/libraries/standard/backoffAlgorithm/backoffAlgorithmFilePaths.cmake )

# RTIO library source files.
set( RTIO_SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/source/core_rtio.c"
     "${CMAKE_CURRENT_LIST_DIR}/source/core_rtio_serializer.c"  
     "${BACKOFF_ALGORITHM_SOURCES}" )

# RTIO library Public Include directories.
set( RTIO_INCLUDE_PUBLIC_DIRS
     "${CMAKE_CURRENT_LIST_DIR}/source/include" )
set( RTIO_INCLUDE_INTERNEL_DIRS
     "${CMAKE_CURRENT_LIST_DIR}/source/internel"
     "${BACKOFF_ALGORITHM_INCLUDE_PUBLIC_DIRS}" )

