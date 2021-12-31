cmake_minimum_required(VERSION 3.10)
project(libmsbtfont VERSION 0.1 LANGUAGES C CXX)

set(LIBRARY_TYPE "STATIC" CACHE STRING "Library type")
set_property(CACHE LIBRARY_TYPE PROPERTY STRINGS "STATIC;SHARED")

if (LIBRARY_TYPE STREQUAL "SHARED")
	set(LIBRARY_TYPE_DEFINE MSBTFONT_SHARED)
endif ()

include(GNUInstallDirs)

add_library(msbtfont ${LIBRARY_TYPE} src/msbtfont.c)
set_target_properties(msbtfont PROPERTIES VERSION 0.1 SOVERSION 0.1)
if (LIBRARY_TYPE STREQUAL "SHARED")
	target_compile_definitions(msbtfont PUBLIC MSBTFONT_SHARED)
	if (WIN32)
		target_compile_definitions(msbtfont PUBLIC MSBTFONT_DEVELOPMENT_DLL)
	endif ()
endif ()
install(TARGETS msbtfont
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES include/msbtfont.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/msbtfont")
