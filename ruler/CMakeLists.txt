add_executable(ScroomRuler)
target_sources(ScroomRuler
        PRIVATE src/main.cc
                src/ruler.cc
                src/ruler.hh)
target_link_libraries(ScroomRuler
        PUBLIC
        ${GTK3_LIBRARIES}
        ${Boost_LIBRARIES})

add_library(ScroomRulerLib)
target_sources(ScroomRulerLib
        PRIVATE src/ruler.cc
                src/ruler.hh)
target_link_libraries(ScroomRulerLib
        PUBLIC ${GTK3_LIBRARIES}
               ${Boost_LIBRARIES})

add_executable(ScroomRuler_test test/ruler-tests.cc)
target_sources(ScroomRuler_test
        PRIVATE test/main.cc)

target_link_libraries(ScroomRuler_test
        PRIVATE ${Boost_LIBRARIES}
                ScroomRulerLib)

add_test(NAME ScroomRuler_test COMMAND ScroomRuler_test)