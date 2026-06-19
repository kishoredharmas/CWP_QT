# CWP Qt — Architecture

## 1. Overview

The Controller Working Position (CWP) Qt application modernises a legacy Air Traffic Control workstation by replacing the proprietary ODS Toolbox UI with Qt 6 (C++) and providing an in-house Recording & Playback Service (RPS replacement). The system follows **Clean Architecture** so that business logic is independent of frameworks, databases, and UI libraries.

---

## 2. Layer Diagram

```mermaid
graph TD
    subgraph UI ["Presentation (Qt)"]
        MW[MainWindow]
        RV[RadarView]
        FSV[FlightStripView]
        FLV[FlightListView]
        PV[PlaybackView]
    end

    subgraph APP ["Application (Use Cases)"]
        DT[DisplayTrackUseCase]
        AF[AssociateFlightPlanUseCase]
        MS[MonitorStaleTracksUseCase]
        SM[SimulateTrackMovementUseCase]
        SD[SeedDemoTracksUseCase]
    end

    subgraph DOM ["Domain (Entities / Interfaces / Services)"]
        TR[Track]
        FP[FlightPlan]
        POS[Position]
        VEL[Velocity]
        IREPO[ITrackRepository]
        IPRES[ITrackPresenter]
        IPLB[IPlaybackService]
        ISECT[ISectorBoundaryService]
    end

    subgraph INFRA ["Infrastructure (IO / Qt / File / Services)"]
        FREPO[FileTrackRepository]
        FPLB[FilePlaybackService]
        PSECT[PolygonSectorBoundaryService]
    end

    MW -->|drives| DT
    MW -->|drives| MS
    MW -->|implements| IPRES
    DT -->|reads| IREPO
    DT -->|drives| IPRES
    AF -->|reads/writes| IREPO
    MS -->|reads/writes| IREPO
    MS -->|notifies| IPRES
    SM -->|reads/writes| IREPO
    SM -->|uses| ISECT
    SD -->|writes| IREPO
    SD -->|uses| ISECT
    FREPO -->|implements| IREPO
    FPLB -->|implements| IPLB
    PSECT -->|implements| ISECT
    FREPO -->|uses| TR
    DT -->|uses| TR
    TR -->|has| POS
    TR -->|has| VEL
```

---

## 3. Use Cases Overview

| Use Case | Responsibility | Triggered By |
|----------|----------------|--------------|
| `DisplayTrackUseCase` | Fetch all tracks and present to UI | Timer (1s interval) |
| `MonitorStaleTracksUseCase` | Evict tracks not updated within 60s | Timer (1s interval) |
| `SimulateTrackMovementUseCase` | Advance aircraft positions along velocity vectors | Timer (simulation) |
| `SeedDemoTracksUseCase` | Generate initial demo tracks at sector boundaries | Application startup |
| `AssociateFlightPlanUseCase` | Correlate flight plan with radar track | User action |

---

## 3. Dependency Rule

```mermaid
graph LR
    UI --> APP
    APP --> DOM
    INFRA --> DOM
    UI --> INFRA
    UI -.->|forbidden| INFRA
```

> Source code dependencies point **inward only**. Domain has zero outward dependencies.

---

## 4. Component Diagram

```mermaid
C4Component
    title CWP Qt — Component View

    Container(cwp, "cwp_qt Executable", "C++23 / Qt 6")

    Component(main, "main()", "Entry point / DI root")
    Component(mw, "MainWindow", "Qt top-level window, ITrackPresenter")
    Component(rv, "RadarView", "Custom QPainter widget")
    Component(dtu, "DisplayTrackUseCase", "Refresh all tracks")
    Component(mstu, "MonitorStaleTracksUseCase", "Evict stale tracks")
    Component(smtu, "SimulateTrackMovementUseCase", "Advance aircraft positions")
    Component(sdtu, "SeedDemoTracksUseCase", "Generate demo tracks")
    Component(afu, "AssociateFlightPlanUseCase", "Correlate FP → Track")
    Component(frepo, "FileTrackRepository", "In-memory map + JSON file")
    Component(fplb, "FilePlaybackService", "Binary file recorder/replayer")
    Component(psect, "PolygonSectorBoundaryService", "Ray-casting boundary detection")

    Rel(main, mw, "creates")
    Rel(main, dtu, "creates & wires")
    Rel(main, mstu, "creates & wires")
    Rel(main, smtu, "creates & wires")
    Rel(main, sdtu, "creates & wires")
    Rel(main, frepo, "creates")
    Rel(main, psect, "creates")
    Rel(mw, rv, "owns")
    Rel(mw, dtu, "triggers on timer")
    Rel(mw, mstu, "triggers on timer")
    Rel(dtu, frepo, "findAll()")
    Rel(dtu, mw, "presentTracks()")
    Rel(mstu, frepo, "findAll() / remove()")
    Rel(mstu, mw, "presentTrackRemoved()")
    Rel(smtu, frepo, "findAll() / save()")
    Rel(smtu, psect, "isInsideSector()")
    Rel(sdtu, frepo, "save()")
    Rel(sdtu, psect, "randomBoundaryPosition()")
    Rel(afu, frepo, "findById() / save()")
```

---

## 5. Class Diagram — Domain

```mermaid
classDiagram
    class Position {
        -double m_latitude
        -double m_longitude
        -int m_altitude
        +latitude() double
        +longitude() double
        +altitude() int
        +create(lat, lon, alt) expected~Position, PositionError~
    }

    class Velocity {
        -double m_speed
        -double m_heading
        -int m_verticalRate
        +speed() double
        +heading() double
        +verticalRate() int
        +create(speed, hdg, vr) expected~Velocity, VelocityError~
    }

    class Track {
        -TrackId m_id
        -Position m_position
        -Velocity m_velocity
        -time_point m_timestamp
        -optional~string~ m_callsign
        -bool m_insideSector
        +id() TrackId
        +position() Position
        +velocity() Velocity
        +callsign() optional~string~
        +isInsideSector() bool
        +associateCallsign(string)
        +setInsideSector(bool)
        +update(Position, Velocity, time_point)
        +isStale(threshold, now) bool
    }

    class FlightPlan {
        -string m_callsign
        -string m_aircraftType
        -string m_squawk
        -int m_assignedLevel
        -vector~string~ m_route
        +callsign() string
        +aircraftType() string
        +squawk() string
        +assignedLevel() int
        +route() vector~string~
        +amendLevel(int)
    }

    class ITrackRepository {
        <<interface>>
        +findAll() vector~Track~
        +findById(TrackId) optional~Track~
        +save(Track)
        +remove(TrackId)
    }

    class ITrackPresenter {
        <<interface>>
        +presentTracks(span~Track~)
        +presentTrackRemoved(TrackId)
    }

    class IPlaybackService {
        <<interface>>
        +startRecording(string) expected~void, PlaybackError~
        +stopRecording()
        +startPlayback(string) expected~void, PlaybackError~
        +stopPlayback()
        +isRecording() bool
        +isPlaying() bool
    }

    class ISectorBoundaryService {
        <<interface>>
        +isInsideSector(Position) bool
        +randomBoundaryPosition() Position
        +sectorCenter() Position
    }

    Track *-- Position
    Track *-- Velocity
```
        +remove(TrackId)
    }

    class ITrackPresenter {
        <<interface>>
        +presentTracks(vector~Track~)
        +presentTrackRemoved(TrackId)
    }

    class IPlaybackService {
        <<interface>>
        +startRecording(string)
        +stopRecording()
        +startPlayback(string)
        +stopPlayback()
        +isRecording() bool
        +isPlaying() bool
    }

    Track *-- Position
    Track *-- Velocity
```

---

## 6. Sequence — Radar Refresh Cycle

```mermaid
sequenceDiagram
    participant Timer as QTimer (1 s)
    participant MW as MainWindow
    participant MST as MonitorStaleTracksUseCase
    participant DT as DisplayTrackUseCase
    participant Repo as FileTrackRepository
    participant RV as RadarView

    Timer->>MW: timeout()
    
    Note over MW,MST: Safety first: evict stale tracks
    MW->>MST: execute()
    MST->>Repo: findAll()
    Repo-->>MST: vector<Track>
    MST->>MST: check staleness
    MST->>Repo: remove(staleIds)
    MST->>MW: presentTrackRemoved(id)
    
    Note over MW,DT: Then refresh display
    MW->>DT: execute()
    DT->>Repo: findAll()
    Repo-->>DT: vector<Track>
    DT->>MW: presentTracks(tracks)
    MW->>RV: updateTracks(tracks)
    RV->>RV: update() → paintEvent()
```

---

## 7. Sequence — Simulation Cycle

```mermaid
sequenceDiagram
    participant Timer as QTimer (1 s)
    participant Main as main()
    participant SM as SimulateTrackMovementUseCase
    participant Repo as FileTrackRepository
    participant Sect as PolygonSectorBoundaryService

    Timer->>Main: timeout()
    Main->>SM: execute(deltaSeconds)
    SM->>Repo: findAll()
    Repo-->>SM: vector<Track>
    
    loop For each track
        SM->>SM: calculate new position
        SM->>Sect: isInsideSector(newPosition)
        Sect-->>SM: bool
        SM->>SM: track.setInsideSector(bool)
        SM->>SM: track.update(position, velocity, now)
        SM->>Repo: save(track)
    end
```

---

## 8. Sequence — Flight Plan Association

```mermaid
sequenceDiagram
    participant Op as Operator
    participant MW as MainWindow
    participant UC as AssociateFlightPlanUseCase
    participant Repo as FileTrackRepository

    Op->>MW: select track + enter callsign
    MW->>UC: execute(trackId, flightPlan)
    UC->>Repo: findById(trackId)
    Repo-->>UC: optional<Track>
    UC->>UC: track.associateCallsign(callsign)
    UC->>Repo: save(track)
    Repo-->>UC: ok
    UC-->>MW: true
```

---

## 9. Deployment Diagram

```mermaid
graph LR
    subgraph Workstation["Controller Workstation (Linux x86-64)"]
        APP[cwp_qt process]
        FS[(cwp_tracks.json\nrecordings/)]
        CFG[config/\nConfiguration.hpp]
    end

    subgraph Radar["Radar Data Source (Future)"]
        ASTERIX[ASTERIX Cat-48 feed]
    end

    ASTERIX -.->|UDP multicast\n(not yet implemented)| APP
    APP <-->|read/write| FS
    APP -->|compile-time| CFG
```

---

## 10. Design Decisions

### 10.1 Clean Architecture

**Decision**: Strict layered architecture with inward-pointing dependencies.

**Rationale**:
- Domain logic is framework-independent (testable without Qt)
- Easy to swap infrastructure (e.g., PostgreSQL instead of JSON files)
- UI changes don't affect business rules
- Supports long-term maintainability

### 10.2 Use Case Pattern

**Decision**: Each feature encapsulated as a dedicated class.

**Rationale**:
- Single Responsibility Principle
- Testable in isolation
- Clear boundaries for code review
- Matches ATC operational procedures (one use case per action)

### 10.3 Domain Services

**Decision**: Geometry and boundary logic extracted into `ISectorBoundaryService`.

**Rationale**:
- Reusable across multiple sectors
- Testable without UI or simulation
- Thread-safe implementation
- Supports different algorithms (polygon, circle, etc.)

### 10.4 Value Objects with Validation

**Decision**: `Position` and `Velocity` validate inputs in factory methods.

**Rationale**:
- Safety-critical: invalid coordinates cannot enter the system
- Fail-fast at data ingestion boundaries
- `std::expected` forces explicit error handling
- Compiler optimizations with `[[assume]]` hints

### 10.5 Sector Status in Track Entity

**Decision**: Track entity stores `m_insideSector` boolean.

**Rationale**:
- Avoids recalculating geometry on every paint cycle
- Simplifies UI logic (color selection based on simple flag)
- Updated atomically with position changes
- Matches ATC mental model (track is "in my sector" or not)

### 10.6 Trail Management in UI Layer

**Decision**: Trail history managed by `RadarView` widget.

**Rationale**:
- Display-specific concern (not domain logic)
- Qt widget ownership simplifies lifecycle management
- Geographic coordinates stored (not screen pixels) for pan/zoom support
- **Note**: Could be extracted to a service if trails need persistence

### 10.7 Configuration as Compile-Time Constants

**Decision**: Configuration in header file (`config/Configuration.hpp`).

**Rationale**:
- Zero runtime overhead
- Compiler can optimize based on constants
- Simple for development/prototyping
- **Trade-off**: Requires recompilation to change parameters
- **Future**: Could support runtime config files if needed

---

## 11. Technology Stack

| Layer | Technology | Version | Rationale |
|-------|-----------|---------|-----------|
| Language | C++ | 23 | Modern features (std::expected, spaceship, ranges) |
| UI Framework | Qt | 6.2+ | Cross-platform, mature, good QPainter performance |
| Build System | CMake | 3.21+ | Industry standard, good IDE support |
| Testing | Google Test | 1.12+ | Mature, well-documented, parameterized tests |
| VCS | Git | 2.x | Standard version control |

---

## 12. Future Enhancements

### Short Term
1. **JSON Serialization**: Implement persistence in `FileTrackRepository`
2. **Unit Test Coverage**: Add tests for all use cases and services
3. **Error Reporting**: Structured error handling with result types

### Medium Term
4. **ASTERIX Feed Integration**: Real radar data ingestion
5. **Multi-Sector Support**: Load sector definitions from config files
6. **User Preferences**: Persist UI settings (colors, zoom level, etc.)

### Long Term
7. **Distributed System**: Multiple CWP instances sharing data
8. **Conflict Detection**: Automated separation assurance
9. **Voice Integration**: Push-to-talk with radio simulation

---

## 13. References

- **Clean Architecture** — Robert C. Martin (Uncle Bob)
- **Domain-Driven Design** — Eric Evans
- **Qt 6 Documentation** — https://doc.qt.io/qt-6/
- **ICAO Annex 11** — Air Traffic Services
- **ASTERIX Specification** — EUROCONTROL
