cmake_minimum_required(VERSION 3.1)

SET(CMAKE_CONFIGURATION_TYPES "Debug;Release")

project (RayTracer)
file(GLOB SOURCES Src/*.cpp Src/*.h)

if(APPLE)
    file(GLOB SOURCES_PLATFORM Src/OSX/*.m Src/OSX/*.mm Src/OSX/*.h)
	set (CMAKE_CXX_STANDARD 11)
	find_library(APPKIT_LIBRARY AppKit)
	set(EXTRA_LIBS ${APPKIT_LIBRARY})
endif()

if(WIN32)
    file(GLOB SOURCES_PLATFORM Src/Windows/*.cpp Src/Windows/*.h)
endif()

add_executable (RayTracer WIN32 ${SOURCES} ${SOURCES_PLATFORM})
target_link_libraries( RayTracer ${EXTRA_LIBS} )
