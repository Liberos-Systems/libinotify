#include "libinotify.hpp"

// Custom formatter for 'fmt' library to handle 'filesystem::path'
namespace fmt {
    template <>
    struct formatter<std::filesystem::path> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

        template <typename FormatContext>
        auto format(const std::filesystem::path& p, FormatContext& ctx) {
            return format_to(ctx.out(), "{}", p.string());
        }
    };
}

namespace inotify
{
    // private
    void Watcher::observeFiles()
    {
        for (const auto& file : watchList) {
            int wd = inotify_add_watch(fd, file.c_str(), IN_ALL_EVENTS);
            if (wd < 0) {
                spdlog::error("Failed to add watch for file: {}", file);
                continue;
            }
            if (verbose) { // Show information if verbose is true
                spdlog::info("Watching file: {}", file);
            }
        }

        if(observerThread.joinable()){
            observerThread.join();
        }
        observerThread = std::thread([this](){ this->observeFiles(); });
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
            if (*it == file) {
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
                auto it = std::find(watchList.begin(), watchList.end(), line.substr(1));
                if (it == watchList.end()) {
                    watchList.push_back(line.substr(1));
                    if (verbose) { // Show information if verbose is true
                        spdlog::info("Added to watchlist: {}", line.substr(1));
                    }
                }
            } else if (line[0] == '-') {
                auto it = std::find(watchList.begin(), watchList.end(), line.substr(1));
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
            if (std::regex_match(it->string(), pattern_regex)) {
                if (verbose) { // Show information if verbose is true
                    spdlog::info("Removed from watchlist: {}", *it);
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
            if (std::regex_match(it->string(), pattern_regex)) {
                if (verbose) { // Show information if verbose is true
                    spdlog::info("Removed from watchlist: {}", *it);
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
        // Using C++20 and recursive function
        std::function<void(const std::filesystem::path&)> traverse = [&](const std::filesystem::path& p) {
            if (std::filesystem::is_directory(p)) {
                // Set the recursive mode flag
                this->recursiveMode = true;
                for (auto& entry : std::filesystem::directory_iterator(p)) {
                    if (std::filesystem::is_directory(entry)) {
                        traverse(entry.path());
                    } else if (std::filesystem::is_regular_file(entry)) {
                        watchList.push_back(entry.path());
                        if (verbose) { // Show information if verbose is true
                            spdlog::info("Added to watchlist: {}", entry.path().string());
                        }
                    }
                }
            } else if (std::filesystem::is_regular_file(p)) {
                spdlog::warn("The provided path is a file, not a directory: {}", p.string());
                watchList.push_back(p);
                if (verbose) { // Show information if verbose is true
                    spdlog::info("Added to watchlist: {}", p.string());
                }
            }
        };

        traverse(std::filesystem::path(path));

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

    bool Watcher::setVerbose(bool value)
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

    std::vector<std::string> Watcher::getCurrentEvents() const
    {
        std::vector<std::string> currentEvents;
        auto fileEventsCopy = this->fileEvents;  // Create a copy of fileEvents
        for (auto& event : fileEventsCopy) {     // Remove const qualifier
            while (!event.second.empty()) {
                currentEvents.push_back(event.second.front());
                event.second.pop();
            }
        }
        return currentEvents;
    }
}



