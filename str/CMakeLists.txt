# str/str
cmake_minimum_required (VERSION 3.15)

if (MSVC)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

MAKE_VERSION_HEADER("${CMAKE_CURRENT_SOURCE_DIR}/version.h" STR "${STR_VERSION}")

if (WIN32)
	set(STR_RESOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/str.rc")
	file(REMOVE "${STR_RESOURCE_FILE}")
	file(TOUCH "${STR_RESOURCE_FILE}")
	include(ResourceMaker)
	CREATE_VERSION_RESOURCE(
		"${STR_RESOURCE_FILE}"
		"${STR_VERSION_MAJOR}"
		"${STR_VERSION_MINOR}"
		"${STR_VERSION_PATCH}"
		"radj307"
		"Commandline string manipulation utility."
		"str"
		"GPLv3"
		"str"
		"str"
	)
	APPEND_ICON_RESOURCE("${STR_RESOURCE_FILE}" "${CMAKE_CURRENT_SOURCE_DIR}/str.ico")
endif()

file(GLOB HEADERS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"*.h*"
)


add_executable(str "main.cpp" "${STR_RESOURCE_FILE}")

# Set properties
set_property(TARGET str PROPERTY CXX_STANDARD 20)
set_property(TARGET str PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET str PROPERTY POSITION_INDEPENDENT_CODE ON)
if (MSVC)
	target_compile_options(str PUBLIC "/Zc:__cplusplus")
endif()

include(PrependEach)
PREPEND_EACH(HEADERS_ABS "${HEADERS}" "${CMAKE_CURRENT_SOURCE_DIR}/")
target_sources(str PUBLIC
	"$<BUILD_INTERFACE:${HEADERS_ABS}>"
	"$<INSTALL_INTERFACE:${HEADERS}>"
)

target_link_libraries(str PUBLIC shared TermAPI strlib optlib filelib)

install(
	TARGETS str
	RUNTIME
	DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
)
