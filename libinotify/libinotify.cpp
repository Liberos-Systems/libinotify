#include "libinotify.hpp"
#include "spdlog/spdlog.h"
#include <regex>

namespace inotify
{
    void Watcher::excludeFile(const std::string& file)
    {
        // Check if the watcher is in recursive mode
        if (!this->recursiveMode) {
            spdlog::error("excludeFile requires the watcher to be in recursive mode.");
            throw std::runtime_error("excludeFile requires the watcher to be in recursive mode.");
        }

        // Exclude the file from the watch list if it's already there
        auto it = std::find(watchList.begin(), watchList.end(), file);
        if (it != watchList.end()) {
            watchList.erase(it);
        }
    }

    void Watcher::fromFile(const std::string& file)
    {
        std::ifstream infile(file);
        std::string line;
        while (std::getline(infile, line)) {
            if (line[0] == '@') {
                auto it = std::find(watchList.begin(), watchList.end(), line.substr(1));
                if (it == watchList.end()) {
                    watchList.push_back(line.substr(1));
                    spdlog::info("Added to watchlist: {}", line.substr(1));
                }
            } else if (line[0] == '-') {
                auto it = std::find(watchList.begin(), watchList.end(), line.substr(1));
                if (it != watchList.end()) {
                    watchList.erase(it);
                    spdlog::info("Removed from watchlist: {}", line.substr(1));
                }
            }
        }
    }

    void Watcher::zero()
    {
        // Implementation of outputting table rows and columns even if all elements are zero
    }

    void Watcher::exclude(const std::string& pattern)
    {
        // Implementation of not processing any events whose filename matches the specified POSIX extended regular expression, case sensitive
        std::regex pattern_regex(pattern);
        for (auto it = watchList.begin(); it != watchList.end(); ) {
            if (std::regex_match(it->string(), pattern_regex)) {
                spdlog::info("Removed from watchlist: {}", it->string());
                it = watchList.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Watcher::excludei(const std::string& pattern)
    {
        // Implementation of not processing any events whose filename matches the specified POSIX extended regular expression, case insensitive
        std::regex pattern_regex(pattern, std::regex::icase);
        for (auto it = watchList.begin(); it != watchList.end(); ) {
            if (std::regex_match(it->string(), pattern_regex)) {
                spdlog::info("Removed from watchlist: {}", it->string());
                it = watchList.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Watcher::recursive()
    {
        // Set the recursive mode flag
        this->recursiveMode = true;

        // Implementation of watching all subdirectories of any directories passed as arguments
        // Using C++23 and generators
        auto generator = [] (const std::filesystem::path& p) -> std::generator<std::filesystem::path> {
            if (std::filesystem::is_directory(p)) {
                for (auto& entry : std::filesystem::recursive_directory_iterator(p)) {
                    co_yield entry.path();
                }
            }
        };

        for (auto& path : generator(this->path)) {
            watchList.push_back(path);
            spdlog::info("Added to watchlist: {}", path.string());
        }
    }

    void Watcher::timeout(int seconds)
    {
        // Implementation of listening only for the specified amount of seconds
    }

    void Watcher::event(const std::string& event)
    {
        // Implementation of listening for specific event(s) only
    }

    void Watcher::ascending(const std::string& event)
    {
        // Implementation of sorting output ascending by event counts for the specified event
    }

    void Watcher::descending(const std::string& event)
    {
        // Implementation of sorting output descending by event counts for the specified event
    }
}

