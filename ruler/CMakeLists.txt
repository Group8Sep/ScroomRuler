add_executable(ScroomRuler)
target_sources(ScroomRuler
        PRIVATE src/main.cc
                src/ruler.cc
                src/ruler.hh)

target_link_libraries(ScroomRuler
        PUBLIC
        ${GTK3_LIBRARIES}
        ${Boost_LIBRARIES})

add_executable(ScroomRuler_test)
target_sources(ScroomRuler_test
        PRIVATE test/main.cc)

target_link_libraries(ScroomRuler_test ${Boost_LIBRARIES})

add_test(NAME ScroomRuler_test COMMAND ScroomRuler_test)