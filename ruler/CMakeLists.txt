add_executable(ScroomRuler)
target_sources(ScroomRuler
        PRIVATE src/main.cc
                src/ruler.cc
                src/ruler.hh)

target_link_libraries(ScroomRuler
        PUBLIC
        ${GTK3_LIBRARIES}
        Boost::system)