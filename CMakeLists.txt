# str
cmake_minimum_required (VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/307lib/307modules")

set(ENV{STR_VERSION} "0.0.0")

include(VersionTag)
GET_VERSION_TAG("${CMAKE_CURRENT_SOURCE_DIR}" STR)

project ("str" VERSION "${STR_VERSION}")

add_subdirectory("307lib")
add_subdirectory ("str")
