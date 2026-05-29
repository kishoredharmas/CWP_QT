# Build GMock + GTest from the system source tree (GMock has no pre-built .a)
add_subdirectory(/usr/src/googletest googletest EXCLUDE_FROM_ALL)

add_executable(cwp_tests
    domain/TrackTest.cpp
    domain/FlightPlanTest.cpp
    application/DisplayTrackUseCaseTest.cpp
    application/AssociateFlightPlanUseCaseTest.cpp
    application/MonitorStaleTracksUseCaseTest.cpp
)

target_link_libraries(cwp_tests PRIVATE
    domain_lib
    application_lib
    gtest
    gmock
    gtest_main
)

target_include_directories(cwp_tests PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/tests
)

include(GoogleTest)
gtest_discover_tests(cwp_tests)
