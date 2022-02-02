# str
cmake_minimum_required (VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/307lib/307modules")

set(ENV{ENV_VERSION} "0.0.0")
include(Autoversion2)
GET_VERSION_2("STR" "${CMAKE_CURRENT_SOURCE_DIR}" MAJOR MINOR PATCH EXTRA)

project ("str" VERSION "${STR_VERSION}")

add_subdirectory("307lib")
add_subdirectory ("str")
