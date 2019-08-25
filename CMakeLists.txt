CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(CMAKE_CONFIGURATION_TYPES "Debug;Release")
SET(CMAKE_CXX_STANDARD 11)

PROJECT(RayTracer)
FILE(GLOB SOURCES Src/*.cpp Src/*.h)

IF(APPLE)
    FILE(GLOB SOURCES_PLATFORM Src/OSX/*.m Src/OSX/*.mm Src/OSX/*.h Src/OSX/*.plist)
	FIND_LIBRARY(APPKIT_LIBRARY AppKit)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
	SET(EXTRA_LIBS ${APPKIT_LIBRARY})
ENDIF()

IF(WIN32)
    FILE(GLOB SOURCES_PLATFORM Src/Windows/*.cpp Src/Windows/*.h)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /fp:fast")

	# Multi-processor compilation support
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

	# Disable warnings on using c functions with char* parameters
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()

ADD_EXECUTABLE(RayTracer WIN32 MACOSX_BUNDLE ${SOURCES} ${SOURCES_PLATFORM})
TARGET_LINK_LIBRARIES(RayTracer ${EXTRA_LIBS})
add_dependencies(RayTracer png_static)
target_include_directories(RayTracer PRIVATE ${CMAKE_SOURCE_DIR}/ThirdParty/libpng ${CMAKE_BINARY_DIR}/ThirdParty/libpng)
target_link_libraries(RayTracer png_static)

IF(APPLE)
    SET_TARGET_PROPERTIES(RayTracer PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Src/OSX/MacOSXBundleInfo.plist)
ENDIF()

FUNCTION(ASSIGN_SOURCE_GROUP)
    FOREACH(_SOURCE IN ITEMS ${ARGN})
        IF (IS_ABSOLUTE "${_SOURCE}")
            FILE(RELATIVE_PATH _SOURCE_REL "${CMAKE_CURRENT_SOURCE_DIR}" "${_SOURCE}")
        ELSE()
            SET(_SOURCE_REL "${_SOURCE}")
        ENDIF()
        GET_FILENAME_COMPONENT(_SOURCE_PATH "${_SOURCE_REL}" PATH)
        STRING(REPLACE "/" "\\" _SOURCE_PATH_IDE "${_SOURCE_PATH}")
        SOURCE_GROUP("${_SOURCE_PATH_IDE}" FILES "${_SOURCE}")
    ENDFOREACH()
ENDFUNCTION(ASSIGN_SOURCE_GROUP)

ASSIGN_SOURCE_GROUP(${SOURCES} ${SOURCES_PLATFORM})

# Add third party libraries
add_subdirectory(ThirdParty/zlib)

# Do not install libpng
set(SKIP_INSTALL_ALL 1)

set(ZLIB_INCLUDE_DIR "${CMAKE_BINARY_DIR}/ThirdParty/zlib" "${CMAKE_SOURCE_DIR}/ThirdParty/zlib")
set(ZLIB_LIBRARY zlibstatic)

add_subdirectory(ThirdParty/libpng)
