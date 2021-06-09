cmake_minimum_required(VERSION 3.19)
project(ScroomRuler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)

# Find GTK3
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

add_definitions(${GTK3_CFLAGS_OTHER})

# Find Boost
set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost REQUIRED COMPONENTS system unit_test_framework)
include_directories(${Boost_INCLUDE_DIR})

enable_testing()

add_subdirectory(ruler)