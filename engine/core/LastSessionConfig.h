#pragma once

#include <fstream>
#include <string>
#include <filesystem>

namespace fresh
{

/**
 * @brief Persists last-session state so that the client/runtime can
 *        automatically load the world most recently saved by the editor.
 *
 * The editor writes this file every time a world is saved.
 * The client reads it on startup and loads that world, so that
 * "any changes made in the editor are reflected in the client
 *  once opened."
 *
 * File location: config/last_session.cfg  (relative to working directory)
 * Format:        simple KEY=VALUE text file
 */
class LastSessionConfig
{
public:
    /**
     * @brief Save the path of the most recently saved world
     * @param worldPath Absolute or relative path to the .world file
     */
    static void saveLastWorldPath(const std::string& worldPath)
    {
        namespace fs = std::filesystem;
        try {
            fs::create_directories("config");
        } catch (...) {
            // best-effort; directory may already exist
        }

        std::ofstream file(configFilePath());
        if (file.is_open()) {
            file << "# Fresh Engine - Last Session\n";
            file << "# Written automatically when the editor saves a world.\n";
            file << "# The client reads this file on startup to load the latest world.\n\n";
            file << "LAST_WORLD_PATH=" << worldPath << "\n";
        }
    }

    /**
     * @brief Load the path of the most recently saved world
     * @return Path to the .world file, or empty string if none found
     */
    static std::string loadLastWorldPath()
    {
        std::ifstream file(configFilePath());
        if (!file.is_open()) {
            return "";
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            const std::string key = "LAST_WORLD_PATH=";
            if (line.rfind(key, 0) == 0) {
                return line.substr(key.size());
            }
        }
        return "";
    }

private:
    static const char* configFilePath()
    {
        return "config/last_session.cfg";
    }
};

} // namespace fresh
