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
    end

    subgraph APP ["Application (Use Cases)"]
        DT[DisplayTrackUseCase]
        AF[AssociateFlightPlanUseCase]
    end

    subgraph DOM ["Domain (Entities / Interfaces)"]
        TR[Track]
        FP[FlightPlan]
        POS[Position]
        VEL[Velocity]
        IREPO[ITrackRepository]
        IPRES[ITrackPresenter]
        IPLB[IPlaybackService]
    end

    subgraph INFRA ["Infrastructure (IO / Qt / File)"]
        FREPO[FileTrackRepository]
        FPLB[FilePlaybackService]
    end

    MW -->|drives| DT
    MW -->|implements| IPRES
    DT -->|reads| IREPO
    DT -->|drives| IPRES
    AF -->|reads/writes| IREPO
    FREPO -->|implements| IREPO
    FPLB -->|implements| IPLB
    FREPO -->|uses| TR
    DT -->|uses| TR
    TR -->|has| POS
    TR -->|has| VEL
```

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

    Container(cwp, "cwp_qt Executable", "C++17 / Qt 6")

    Component(main, "main()", "Entry point / DI root")
    Component(mw, "MainWindow", "Qt top-level window, ITrackPresenter")
    Component(rv, "RadarView", "Custom QPainter widget")
    Component(dtu, "DisplayTrackUseCase", "Refresh all tracks")
    Component(afu, "AssociateFlightPlanUseCase", "Correlate FP → Track")
    Component(frepo, "FileTrackRepository", "In-memory map + JSON file")
    Component(fplb, "FilePlaybackService", "Binary file recorder/replayer")

    Rel(main, mw, "creates")
    Rel(main, dtu, "creates & wires")
    Rel(main, frepo, "creates")
    Rel(mw, rv, "owns")
    Rel(mw, dtu, "triggers on timer")
    Rel(dtu, frepo, "findAll()")
    Rel(dtu, mw, "presentTracks()")
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
    }

    class Velocity {
        -double m_speed
        -double m_heading
        -int m_verticalRate
        +speed() double
        +heading() double
        +verticalRate() int
    }

    class Track {
        -TrackId m_id
        -Position m_position
        -Velocity m_velocity
        -time_point m_timestamp
        -optional~string~ m_callsign
        +id() TrackId
        +position() Position
        +velocity() Velocity
        +callsign() optional~string~
        +associateCallsign(string)
        +update(Position, Velocity, time_point)
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
    participant Timer as QTimer (4 s)
    participant MW as MainWindow
    participant UC as DisplayTrackUseCase
    participant Repo as FileTrackRepository
    participant RV as RadarView

    Timer->>MW: timeout()
    MW->>UC: execute()
    UC->>Repo: findAll()
    Repo-->>UC: vector<Track>
    UC->>MW: presentTracks(tracks)
    MW->>RV: updateTracks(tracks)
    RV->>RV: update() → paintEvent()
```

---

## 7. Sequence — Flight Plan Association

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

## 8. Deployment Diagram

```mermaid
graph LR
    subgraph Workstation["Controller Workstation (Linux x86-64)"]
        APP[cwp_qt process]
        FS[(cwp_tracks.json\nrecordings/)]
    end

    subgraph Radar["Radar Data Source"]
        ASTERIX[ASTERIX Cat-48 feed]
    end

    ASTERIX -->|UDP multicast| APP
    APP <-->|read/write| FS
```
