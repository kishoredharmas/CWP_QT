# CWP Qt — Controller Working Position

Modern Air Traffic Control workstation built with Qt 6 and Clean Architecture principles.

## Overview

The Controller Working Position (CWP) Qt application modernizes legacy ATC workstations by replacing proprietary UI frameworks with Qt 6 (C++) and providing an in-house Recording & Playback Service. The system follows **Clean Architecture** to ensure business logic is independent of frameworks, databases, and UI libraries.

## Features

- **Real-time Radar Display**: 2D plan view with aircraft symbols, velocity vectors, and flight trails
- **Flight Plan Association**: Correlate radar tracks with filed flight plans
- **Sector Boundary Management**: Visual indication when aircraft enter/exit controlled airspace
- **Stale Track Detection**: Automatic removal of outdated radar returns (safety-critical)
- **Recording & Playback**: Session capture for training and incident analysis
- **Multi-view Interface**: Radar, flight strips, flight list, and playback controls

## Architecture

```
┌─────────────────────────────────────┐
│           UI (Qt Widgets)            │ ← Presentation Layer
├─────────────────────────────────────┤
│     Application (Use Cases)          │ ← Business Rules
├─────────────────────────────────────┤
│  Domain (Entities + Interfaces)      │ ← Core Business Logic
├─────────────────────────────────────┤
│  Infrastructure (File, Services)     │ ← External Concerns
└─────────────────────────────────────┘
```

The codebase strictly follows the **Dependency Rule**: source code dependencies point inward only. The domain layer has zero outward dependencies.

See [docs/architecture.md](docs/architecture.md) for detailed architecture documentation.

## Building

### Prerequisites

- C++23 compiler (GCC 12+ or Clang 15+)
- CMake 3.21+
- Qt 6.2+
- Google Test (for unit tests)

### Build Instructions

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Run
./build/cwp_qt

# Run tests
cd build && ctest --output-on-failure
```

## Project Structure

```
CWP_QT/
├── application/         # Use cases (business rules orchestration)
│   └── use_cases/       # DisplayTrack, SimulateMovement, SeedDemoTracks, etc.
├── domain/              # Core business logic (framework-independent)
│   ├── entities/        # Track, FlightPlan
│   ├── value_objects/   # Position, Velocity
│   ├── interfaces/      # Repository, Presenter, Service interfaces
│   ├── services/        # Domain service interfaces
│   └── errors/          # Domain error types
├── infrastructure/      # Framework & I/O implementations
│   ├── repositories/    # FileTrackRepository
│   ├── playback/        # FilePlaybackService
│   └── services/        # PolygonSectorBoundaryService
├── ui/                  # Qt presentation layer
│   ├── MainWindow.*     # Top-level window & presenter
│   ├── RadarView.*      # Custom radar display widget
│   ├── FlightStripView.*
│   ├── FlightListView.*
│   └── PlaybackView.*
├── config/              # Configuration constants
├── tests/               # Unit tests (GTest)
├── docs/                # Architecture & requirements documentation
└── main.cpp             # Application entry point (composition root)
```

## Key Design Patterns

- **Repository Pattern**: Abstract data access through `ITrackRepository`
- **Use Case Pattern**: Each feature encapsulated as a dedicated class
- **Dependency Injection**: All dependencies provided via constructor
- **Strategy Pattern**: Pluggable algorithms (e.g., `ISectorBoundaryService`)
- **Observer Pattern**: Presenter interface for UI updates

## Configuration

Key parameters can be tuned in [`config/Configuration.hpp`](config/Configuration.hpp):

- `k_simulationIntervalMs` — Aircraft update rate
- `k_speedMultiplier` — Visual speed adjustment
- `k_maxTrailPoints` — Trail history length
- `k_staleTimeout` — Track staleness threshold

## Safety Features

The system implements several safety-critical features:

1. **Stale Track Eviction**: Automatically removes tracks not updated within 60 seconds
2. **Validated Value Objects**: `Position` and `Velocity` enforce domain constraints
3. **Boundary Checking**: Prevents invalid geographic coordinates from entering the system
4. **Thread-Safe Repository**: Concurrent read/write access with shared mutexes

## Testing

```bash
# Run all tests
cd build && ctest

# Run specific test suite
./build/tests/cwp_tests --gtest_filter="TrackTest.*"

# Generate coverage report (requires lcov)
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build build
ctest --test-dir build
lcov --capture --directory build --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

## Documentation

- [Architecture](docs/architecture.md) — Detailed system design and diagrams
- [Requirements](docs/requirements.md) — Functional and non-functional requirements
- [Design Review](docs/DESIGN_REVIEW.md) — Latest refactoring improvements

## Contributing

1. Follow Clean Architecture principles
2. Keep the domain layer framework-independent
3. Write unit tests for new use cases
4. Update documentation when adding features
5. Use C++23 features where appropriate

## License

Proprietary — Internal ATC system

