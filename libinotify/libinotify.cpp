#include "libinotify.hpp"
#include "spdlog/spdlog.h"

namespace inotify
{
    // private
    void Watcher::observeFiles()
    {
        for (const auto& file : watchList) {
            int wd = inotify_add_watch(fd, file.first.c_str(), IN_ALL_EVENTS);
            if (wd < 0) {
                spdlog::error("Failed to add watch for file: {}", file.first);
                continue;
            }
            if (verbose) { // Show information if verbose is true
                spdlog::info("Watching file: {}", file.first);
            }
        }

        if(observerThread.joinable()){
            observerThread.join();
        }
        observerThread = std::jthread([this](){ this->observeFiles(); });
    }
    
    // public
    Watcher::Watcher()
    {
        fd = 0;
        try {
            #ifdef NDEBUG
                spdlog::set_level(spdlog::level::info); // Set global log level to info for release version
                spdlog::info("Log level set to info for release version");
                verbose = false;
            #else
                spdlog::set_level(spdlog::level::debug); // Set global log level to debug for developer version
                spdlog::info("Log level set to debug for developer version");
                verbose = true;
            #endif
        } catch (const spdlog::spdlog_ex& ex) {
            spdlog::warn("Failed to set log level: {}", ex.what());
        }

        fd = inotify_init();
        if (fd < 0) {
            spdlog::error("Failed to initialize inotify.");
            throw std::runtime_error("Failed to initialize inotify.");
        }

    }

    Watcher::~Watcher()
    {
        spdlog::warn("Obiekt został usunięty"); // Log warning that the object has been deleted
        spdlog::shutdown(); // Stop logging
    }

    void Watcher::excludeFile(const std::string& file)
    {
        // Check if the watcher is in recursive mode
        if (!this->recursiveMode) {
            spdlog::error("excludeFile requires the watcher to be in recursive mode.");
            throw std::runtime_error("excludeFile requires the watcher to be in recursive mode.");
        }

        // Exclude the file from the watch list if it's already there
        auto it = watchList.begin();
        while (it != watchList.end()) {
            if (it->first == file) {
                it = watchList.erase(it);
            } else {
                ++it;
            }
        }

        if(observerThread.joinable()){
            observerThread.join();
        }
        observerThread = std::thread([this](){ this->observeFiles(); });
    }

    void Watcher::fromFile(const std::string& file)
    {
        std::ifstream infile(file);
        std::string line;
        while (std::getline(infile, line)) {
            if (line[0] == '@') {
                auto it = watchList.find(line.substr(1));
                if (it == watchList.end()) {
                    watchList[line.substr(1)] = std::queue<std::string>();
                    if (verbose) { // Show information if verbose is true
                        spdlog::info("Added to watchlist: {}", line.substr(1));
                    }
                }
            } else if (line[0] == '-') {
                auto it = watchList.find(line.substr(1));
                if (it != watchList.end()) {
                    watchList.erase(it);
                    if (verbose) { // Show information if verbose is true
                        spdlog::info("Removed from watchlist: {}", line.substr(1));
                    }
                }
            }
        }

        if(observerThread.joinable()){
            observerThread.join();
        }
        observerThread = std::thread([this](){ this->observeFiles(); });
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
            if (std::regex_match(it->first, pattern_regex)) {
                if (verbose) { // Show information if verbose is true
                    spdlog::info("Removed from watchlist: {}", it->first);
                }
                it = watchList.erase(it);
            } else {
                ++it;
            }
        }

        if(observerThread.joinable()){
            observerThread.join();
        }
        observerThread = std::thread([this](){ this->observeFiles(); });
    }

    void Watcher::excludei(const std::string& pattern)
    {
        // Implementation of not processing any events whose filename matches the specified POSIX extended regular expression, case insensitive
        std::regex pattern_regex(pattern, std::regex::icase);
        for (auto it = watchList.begin(); it != watchList.end(); ) {
            if (std::regex_match(it->first, pattern_regex)) {
                if (verbose) { // Show information if verbose is true
                    spdlog::info("Removed from watchlist: {}", it->first);
                }
                it = watchList.erase(it);
            } else {
                ++it;
            }
        }

        if(observerThread.joinable()){
            observerThread.join();
        }
        observerThread = std::thread([this](){ this->observeFiles(); });
    }

    void Watcher::recursive(const std::string& path)
    {
        // Implementation of watching all subdirectories of any directories passed as arguments
        // Using C++23 and generators
        auto generator = [] (const std::filesystem::path& p) -> std::generator<std::filesystem::path> {
            if (std::filesystem::is_directory(p)) {
                // Set the recursive mode flag
                this->recursiveMode = true;
                for (auto& entry : std::filesystem::recursive_directory_iterator(p)) {
                    co_yield entry.path();
                }
            } else if (std::filesystem::is_regular_file(p)) {
                spdlog::warn("The provided path is a file, not a directory: {}", p.string());
                co_yield p;
            }
        };

        for (auto& path : generator(this->path)) {
            watchList[path.string()] = std::queue<std::string>();
            if (verbose) { // Show information if verbose is true
                spdlog::info("Added to watchlist: {}", path.string());
            }
        }

        if(observerThread.joinable()){
            observerThread.join();
        }
        observerThread = std::thread([this](){ this->observeFiles(); });
    }

    void Watcher::timeout(int seconds)
    {
        // Implementation of listening only for the specified amount of seconds
        spdlog::info("Timer set for {} seconds", seconds);
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        spdlog::info("Timer of {} seconds has elapsed", seconds);
        // Ensure the object is destroyed after the timer ends
        if(observerThread.joinable()){
            observerThread.join();
        }
        this->~Watcher();
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

    bool Watcher::getVerbose() const
    {
        bool verboseValue = this->verbose;
        spdlog::info("Verbose mode is currently set to: {}", verboseValue);
        return verboseValue;
    }

    void Watcher::setVerbose(bool value)
    {
        if (!std::is_same<decltype(value), bool>::value) {
            spdlog::error("Invalid argument. Expected a boolean value.");
            throw std::invalid_argument("Invalid argument. Expected a boolean value.");
        }
        this->verbose = value;
        getVerbose();
        if (value) {
            spdlog::set_level(spdlog::level::info); // Enable logging at info level
        } else {
            spdlog::set_level(spdlog::level::off); // Disable logging
        }
    }

    std::map<std::string, std::queue<std::string>> Watcher::getCurrentEvents() const
    {
        std::map<std::string, std::queue<std::string>> currentEvents = this->fileEvents;
        this->fileEvents.clear();
        return currentEvents;
    }
}



