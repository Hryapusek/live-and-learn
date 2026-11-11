cmake_minimum_required(VERSION 3.16)
project(gstreamer_cpp_intro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)

add_executable(gst_intro main.cpp)
target_link_libraries(gst_intro PRIVATE PkgConfig::GSTREAMER)
