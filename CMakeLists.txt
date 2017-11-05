cmake_minimum_required(VERSION 3.10)

project (RayTracer)

file(GLOB SOURCES "Src/*.cpp")

add_executable (RayTracer WIN32 ${SOURCES})
