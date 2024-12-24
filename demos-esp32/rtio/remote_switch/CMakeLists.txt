# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# set("IDF_TARGET" esp32c6 CACHE INTERNAL "target platform.")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(remote_switch)

target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "${PLATFORM_DEMOS_ROOT_DIR}/certificates/RTIORootCA.crt" TEXT)
