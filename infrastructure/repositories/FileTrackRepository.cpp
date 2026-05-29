#include "infrastructure/repositories/FileTrackRepository.hpp"

#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <utility>

namespace cwp::infrastructure {

FileTrackRepository::FileTrackRepository(const std::string& filePath)
    : m_filePath{filePath}
{
    loadFromFile(); // called before threads start — no lock needed.
}

std::vector<std::shared_ptr<domain::Track>>
FileTrackRepository::findAll() const
{
    std::shared_lock lock{m_mutex}; // multiple concurrent readers allowed
    std::vector<std::shared_ptr<domain::Track>> result;
    result.reserve(m_tracks.size());
    for (const auto& [id, track] : m_tracks) {
        result.push_back(track);
    }
    return result;
}

std::optional<std::shared_ptr<domain::Track>>
FileTrackRepository::findById(domain::TrackId id) const
{
    std::shared_lock lock{m_mutex};
    const auto it = m_tracks.find(id);
    if (it == m_tracks.end()) {
        return std::nullopt;
    }
    return it->second;
}

void FileTrackRepository::save(std::shared_ptr<domain::Track> track)
{
    std::unique_lock lock{m_mutex};
    m_tracks[track->id()] = std::move(track);
    persistToFile();
}

void FileTrackRepository::remove(domain::TrackId id)
{
    std::unique_lock lock{m_mutex};
    m_tracks.erase(id);
    persistToFile();
}

void FileTrackRepository::loadFromFile()
{
    // Absent file is not an error (fresh session — FR-42).
    std::ifstream file{m_filePath};
    if (!file.is_open()) {
        return;
    }
    // TODO(FR-41): deserialise JSON track records into m_tracks.
    //              Requires a JSON library (e.g. nlohmann/json).
}

void FileTrackRepository::persistToFile() const
{
    // Caller must hold the exclusive lock before calling this.
    std::ofstream file{m_filePath, std::ios::trunc};
    if (!file.is_open()) {
        return; // Non-fatal: best-effort persistence.
    }
    // TODO(FR-40): serialise m_tracks to JSON.
    //              Requires a JSON library (e.g. nlohmann/json).
}

} // namespace cwp::infrastructure
