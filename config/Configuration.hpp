#pragma once

#include <chrono>

namespace cwp::config {

/**
 * @file Configuration.hpp
 * @brief Centralized configuration constants for the CWP Qt application.
 *
 * All configurable parameters are defined here as compile-time constants.
 * This provides zero runtime overhead and enables compiler optimizations.
 * 
 * To change parameters: modify values below and rebuild the application.
 * 
 * @note For runtime configuration support, see Future Enhancements in
 *       docs/architecture.md section 12.
 */

/// ── Simulation Configuration ────────────────────────────────────────────────

/// Simulation update interval in milliseconds
constexpr int k_simulationIntervalMs = 1000;

/// Speed multiplier for visualization (makes aircraft movement more visible)
constexpr double k_speedMultiplier = 5.0;

/// Number of demo tracks to seed on startup
constexpr std::size_t k_demoTrackCount = 12;

/// ── Display Configuration ───────────────────────────────────────────────────

/// UI refresh interval in milliseconds (radar cycle)
constexpr int k_refreshIntervalMs = 1000;

/// Maximum number of trail points per aircraft
constexpr int k_maxTrailPoints = 5;

/// Trail dot radius in pixels
constexpr int k_trailDotRadius = 1;

/// Minimum distance between trail points (degrees ~= 5km)
constexpr double k_trailSpacingDegrees = 0.045;

/// ── Safety Configuration ────────────────────────────────────────────────────

/// Stale track timeout (tracks not updated within this time are evicted)
using namespace std::chrono_literals;
constexpr std::chrono::seconds k_staleTimeout = 60s;

} // namespace cwp::config
