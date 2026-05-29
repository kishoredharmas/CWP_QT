# CWP Qt — Software Requirements Specification

**Document version:** 1.0  
**Project:** Controller Working Position — Qt Modernisation  
**Date:** 2026-05-29  

---

## 1. Introduction

### 1.1 Purpose

This document defines the functional and non-functional requirements for the CWP Qt application, which replaces the legacy ODS Toolbox UI and the Recording & Playback System (RPS) with modern C++17/Qt 6 components.

### 1.2 Scope

The system provides a graphical radar display workstation for Air Traffic Controllers (ATCo). It receives radar track data, associates flight plans, and supports session recording and playback.

### 1.3 Definitions

| Term | Definition |
|------|-----------|
| CWP | Controller Working Position — the ATC operator workstation |
| Track | A radar return representing an aircraft |
| Flight Plan | Filed ATC plan containing callsign, route, and assigned level |
| FL | Flight Level (altitude in hundreds of feet, e.g. FL350 = 35 000 ft) |
| RPS | Recording & Playback System (legacy component being replaced) |
| SSR | Secondary Surveillance Radar |
| Squawk | 4-digit octal SSR transponder code |

---

## 2. Functional Requirements

### 2.1 Radar Display

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-01 | The system **shall** display all active radar tracks on a 2-D plan view. | Must |
| FR-02 | Each track symbol **shall** be rendered as a cross at the projected lat/lon position. | Must |
| FR-03 | Each track label **shall** show: callsign (or `????`), flight level, and ground speed. | Must |
| FR-04 | The display **shall** refresh automatically at intervals of 4 seconds (one radar rotation). | Must |
| FR-05 | The system **shall** remove the track symbol when a track is dropped. | Must |
| FR-06 | The display **shall** draw range rings at configurable intervals. | Should |

### 2.2 Track Management

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-10 | The system **shall** maintain an in-memory set of active tracks keyed by `TrackId`. | Must |
| FR-11 | Each track **shall** carry: position (lat, lon, altitude), velocity (speed, heading, vertical rate), and timestamp. | Must |
| FR-12 | An existing track **shall** be updatable with new position, velocity, and timestamp. | Must |
| FR-13 | A track **may** be associated with at most one callsign from a flight plan. | Must |

### 2.3 Flight Plan Association

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-20 | The system **shall** allow an ATCo to associate a flight plan with a radar track by specifying the track ID. | Must |
| FR-21 | On successful association, the track label **shall** display the flight plan callsign. | Must |
| FR-22 | If the track ID does not exist, the association **shall** fail gracefully and return a failure indicator. | Must |
| FR-23 | A flight plan **shall** carry: callsign, ICAO aircraft type, squawk code, assigned level, and route waypoints. | Must |
| FR-24 | An ATCo **shall** be able to amend the assigned flight level of a flight plan at any time. | Must |

### 2.4 Recording & Playback

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-30 | The system **shall** support recording of ATC session data to a file. | Must |
| FR-31 | The system **shall** support replay of a previously recorded session file. | Must |
| FR-32 | Only one recording or playback session **shall** be active at a time. | Must |
| FR-33 | Starting a new recording **shall** automatically stop any active prior recording. | Must |
| FR-34 | The recording file path **shall** be configurable at session start. | Must |
| FR-35 | If the target file cannot be opened, the system **shall** throw a recoverable error. | Must |

### 2.5 Persistence

| ID | Requirement | Priority |
|----|-------------|----------|
| FR-40 | The track repository **shall** persist its state to a JSON file after every mutation. | Should |
| FR-41 | On startup, the repository **shall** load any existing track data from the backing file. | Should |
| FR-42 | Absence of the backing file at startup **shall not** be treated as an error. | Must |

---

## 3. Non-Functional Requirements

### 3.1 Performance

| ID | Requirement |
|----|-------------|
| NFR-01 | The radar display **shall** render up to 300 simultaneous tracks without frame drops below 25 fps. |
| NFR-02 | Track refresh latency from data-ready to screen-update **shall** be ≤ 200 ms. |

### 3.2 Reliability

| ID | Requirement |
|----|-------------|
| NFR-10 | The system **shall** operate continuously for ≥ 8 hours without memory leaks (verified by Valgrind). |
| NFR-11 | No unhandled exception **shall** propagate to `main()` under nominal operating conditions. |

### 3.3 Maintainability

| ID | Requirement |
|----|-------------|
| NFR-20 | All public classes and functions **shall** carry Doxygen-style documentation. |
| NFR-21 | Cyclomatic complexity of any function **shall** not exceed 10. |
| NFR-22 | Test coverage of domain and application layers **shall** be ≥ 90 % (branch coverage). |

### 3.4 Portability

| ID | Requirement |
|----|-------------|
| NFR-30 | The codebase **shall** compile without warnings on GCC ≥ 12 and Clang ≥ 15 with `-Wall -Wextra -Wpedantic`. |
| NFR-31 | The system **shall** target Linux x86-64 (primary) with no platform-specific API calls outside the infrastructure layer. |

### 3.5 Security

| ID | Requirement |
|----|-------------|
| NFR-40 | No raw owning pointers **shall** be used; `std::shared_ptr` / `std::unique_ptr` **shall** be preferred. |
| NFR-41 | File paths supplied at runtime **shall** be validated against a whitelist of allowed directories before use. |

---

## 4. Constraints

- C++17 or newer; Qt 6 framework.
- Build system: CMake ≥ 3.21.
- Unit testing framework: Google Test / Google Mock.
- Code must comply with MISRA-inspired rules: no magic numbers, const-correct, always-initialised variables.

---

## 5. Traceability Matrix

| Requirement | Implementation |
|-------------|---------------|
| FR-01–FR-05 | `ui/RadarView`, `application/use_cases/DisplayTrackUseCase` |
| FR-10–FR-13 | `domain/entities/Track`, `domain/value_objects/Position`, `Velocity` |
| FR-20–FR-24 | `application/use_cases/AssociateFlightPlanUseCase`, `domain/entities/FlightPlan` |
| FR-30–FR-35 | `infrastructure/playback/FilePlaybackService` |
| FR-40–FR-42 | `infrastructure/repositories/FileTrackRepository` |
| NFR-20      | Doxygen comments throughout `domain/` and `application/` |
| NFR-22      | `tests/` (GTest suite) |
