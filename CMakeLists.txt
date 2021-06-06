cmake_minimum_required(VERSION 3.19)
project(ScroomRuler)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(External)

set(SOURCE_FILES
        src/main.cpp
        src/ruler.cc
        src/ruler.hh
)
add_executable(rulertest ${SOURCE_FILES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

add_definitions(${GTK3_CFLAGS_OTHER})

target_link_libraries(rulertest
        PUBLIC
        ${GTK3_LIBRARIES}
        Boost::system
)