cmake_minimum_required(VERSION 3.21)
project(CWP_QT VERSION 1.0.0 LANGUAGES CXX)

# ── C++ standard ────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Qt automation ────────────────────────────────────────────────────────────
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ── Qt6 ─────────────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# ── Global include root ──────────────────────────────────────────────────────
include_directories(${CMAKE_SOURCE_DIR})

# ── Domain library ───────────────────────────────────────────────────────────
add_library(domain_lib STATIC
    domain/entities/Track.cpp
    domain/entities/FlightPlan.cpp
)
target_include_directories(domain_lib PUBLIC ${CMAKE_SOURCE_DIR})
target_compile_options(domain_lib PRIVATE -Wall -Wextra -Wpedantic)

# ── Application library ──────────────────────────────────────────────────────
add_library(application_lib STATIC
    application/use_cases/DisplayTrackUseCase.cpp
    application/use_cases/AssociateFlightPlanUseCase.cpp    application/use_cases/MonitorStaleTracksUseCase.cpp)
target_link_libraries(application_lib PUBLIC domain_lib)
target_compile_options(application_lib PRIVATE -Wall -Wextra -Wpedantic)

# ── Infrastructure library ───────────────────────────────────────────────────
add_library(infrastructure_lib STATIC
    infrastructure/repositories/FileTrackRepository.cpp
    infrastructure/playback/FilePlaybackService.cpp
)
target_link_libraries(infrastructure_lib PUBLIC domain_lib Qt6::Core)
target_compile_options(infrastructure_lib PRIVATE -Wall -Wextra -Wpedantic)

# ── Main executable ──────────────────────────────────────────────────────────
add_executable(cwp_qt
    main.cpp
    ui/MainWindow.cpp
    ui/RadarView.cpp
)
target_link_libraries(cwp_qt PRIVATE
    domain_lib
    application_lib
    infrastructure_lib
    Qt6::Core
    Qt6::Widgets
)
target_compile_options(cwp_qt PRIVATE -Wall -Wextra -Wpedantic)

# ── Tests ────────────────────────────────────────────────────────────────────
enable_testing()
add_subdirectory(tests)
