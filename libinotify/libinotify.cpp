#include "libinotify.hpp"

namespace inotify
{
    // private
    void Watcher::observeFiles()
    {
        for (const auto &file : watch_list_)
        {
            int wd = inotify_add_watch(fd_, file.c_str(), IN_ALL_EVENTS);
            if (wd < 0)
            {
                spdlog::error("Failed to add watch for file: {}", file);
                continue;
            }
            if (verbose_)
            {
                // Show information if verbose is true
                spdlog::info("Watching file: {}", file);
            }
        }
    }

    // public
    Watcher::Watcher()
    {
        this->enable();

        fd_ = NULL;
        try
        {
            #ifdef NDEBUG
                spdlog::set_level(spdlog::level::info); // Set global log level to info for release version
                spdlog::info("Log level set to info for release version");
                verbose_ = false;
            #else
                spdlog::set_level(spdlog::level::debug); // Set global log level to debug for developer version
                spdlog::info("Log level set to debug for developer version");
                verbose_ = true;
            #endif
        }
        catch (const spdlog::spdlog_ex &ex)
        {
            spdlog::warn("Failed to set log level: {}", ex.what());
        }

        fd_ = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
        if (fd_ < 0)
        {
            switch(errno)
            {
                case EINVAL:
                    spdlog::error("Invalid value specified in flags for inotify_init1.");
                    throw std::invalid_argument("Invalid value specified in flags for inotify_init1.");
                case EMFILE:
                    spdlog::error("User limit on total number of inotify instances has been reached.");
                    throw std::runtime_error("User limit on total number of inotify instances has been reached.");
                case ENFILE:
                    spdlog::error("System-wide limit on total number of open files has been reached.");
                    throw std::runtime_error("System-wide limit on total number of open files has been reached.");
                case ENOMEM:
                    spdlog::error("Insufficient kernel memory is available.");
                    throw std::runtime_error("Insufficient kernel memory is available.");
                default:
                    spdlog::error("Failed to initialize inotify.");
                    throw std::runtime_error("Failed to initialize inotify.");
            }
        }

        run_watcher_thread_ = true;
        if (std::this_thread::get_id() != observer_thread_.get_id() && observer_thread_.joinable())
        {
            observer_thread_.join();
        }

        observer_thread_ = std::thread([this]()
        {
            while (run_watcher_thread_) {
                this->observeFiles();
            } 
        });
    }
    
    void Watcher::excludeFile(const std::string &file)
    {
        // Check if the watcher is in recursive mode
        if (!this->recursive_mode_)
        {
            spdlog::error("excludeFile requires the watcher to be in recursive mode.");
            throw std::runtime_error("excludeFile requires the watcher to be in recursive mode.");
        }

        // Exclude the file from the watch list if it's already there
        auto it = watch_list_.begin();
        while (it != watch_list_.end())
        {
            if (*it == file)
            {
                it = watch_list_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void Watcher::fromFile(const std::string &file)
    {
        std::ifstream infile(file);
        std::string line;
        while (std::getline(infile, line))
        {
            if (line[0] == '@')
            {
                auto it = std::find(watch_list_.begin(), watch_list_.end(), line.substr(1));
                if (it == watch_list_.end())
                {
                    watch_list_.push_back(line.substr(1));
                    if (verbose_)
                    { 
                        // Show information if verbose is true
                        spdlog::info("Added to watchlist: {}", line.substr(1));
                    }
                }
            }
            else if (line[0] == '-')
            {
                auto it = std::find(watch_list_.begin(), watch_list_.end(), line.substr(1));
                if (it != watch_list_.end())
                {
                    watch_list_.erase(it);
                    if (verbose_)
                    { 
                        // Show information if verbose is true
                        spdlog::info("Removed from watchlist: {}", line.substr(1));
                    }
                }
            }
        }
    }

    nlohmann::json Watcher::watch() const
    {
        nlohmann::json emptyJson;
        emptyJson["date"] = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        return emptyJson;
    }

    //syf

    void Watcher::zero()
    {
        // Implementation of outputting table rows and columns even if all elements are zero
    }

    void Watcher::exclude(const std::string &pattern)
    {
        // Implementation of not processing any events whose filename matches the specified POSIX extended regular expression, case sensitive
        std::regex pattern_regex(pattern);
        for (auto it = watch_list_.begin(); it != watch_list_.end();)
        {
            if (std::regex_match(it->string(), pattern_regex))
            {
                if (verbose_)
                { // Show information if verbose is true
                    spdlog::info("Removed from watchlist: {}", *it);
                }
                it = watch_list_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void Watcher::excludei(const std::string &pattern)
    {
        // Implementation of not processing any events whose filename matches the specified POSIX extended regular expression, case insensitive
        std::regex pattern_regex(pattern, std::regex::icase);
        for (auto it = watch_list_.begin(); it != watch_list_.end();)
        {
            if (std::regex_match(it->string(), pattern_regex))
            {
                if (verbose_)
                { // Show information if verbose is true
                    spdlog::info("Removed from watchlist: {}", *it);
                }
                it = watch_list_.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void Watcher::recursive(const std::string &path)
    {
        // Implementation of watching all subdirectories of any directories passed as arguments
        // Using C++20 and recursive function
        std::function<void(const std::filesystem::path &)> traverse = [&](const std::filesystem::path &p)
        {
            if (std::filesystem::is_directory(p))
            {
                // Set the recursive mode flag
                this->recursive_mode_ = true;
                for (auto &entry : std::filesystem::directory_iterator(p))
                {
                    if (std::filesystem::is_directory(entry))
                    {
                        traverse(entry.path());
                    }
                    else if (std::filesystem::is_regular_file(entry))
                    {
                        watch_list_.push_back(entry.path());
                        if (verbose_)
                        { // Show information if verbose is true
                            spdlog::info("Added to watchlist: {}", entry.path().string());
                        }
                    }
                }
            }
            else if (std::filesystem::is_regular_file(p))
            {
                spdlog::warn("The provided path is a file, not a directory: {}", p.string());
                watch_list_.push_back(p);
                if (verbose_)
                { // Show information if verbose is true
                    spdlog::info("Added to watchlist: {}", p.string());
                }
            }
        };

        traverse(std::filesystem::path(path));
    }

    void Watcher::timeout(int seconds)
    {
        // Implementation of listening only for the specified amount of seconds
        spdlog::info("Timer set for {} seconds", seconds);
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        spdlog::info("Timer of {} seconds has elapsed", seconds);
        // Ensure the object is destroyed after the timer ends
        if (observer_thread_.joinable())
        {
            observer_thread_.join();
        }
        this->disable();
    }

    void Watcher::event(const std::string &event)
    {
        // Implementation of listening for specific event(s) only
    }

    void Watcher::ascending(const std::string &event)
    {
        // Implementation of sorting output ascending by event counts for the specified event
    }

    void Watcher::descending(const std::string &event)
    {
        // Implementation of sorting output descending by event counts for the specified event
    }

    bool Watcher::getVerbose() const
    {
        bool verboseValue = this->verbose_;
        spdlog::info("Verbose mode is currently set to: {}", verboseValue);
        return verboseValue;
    }

    bool Watcher::setVerbose(bool value)
    {
        if (!std::is_same<decltype(value), bool>::value)
        {
            spdlog::error("Invalid argument. Expected a boolean value.");
            throw std::invalid_argument("Invalid argument. Expected a boolean value.");
        }
        this->verbose_ = value;
        getVerbose();
        if (value)
        {
            spdlog::set_level(spdlog::level::info); // Enable logging at info level
        }
        else
        {
            spdlog::set_level(spdlog::level::off); // Disable logging
        }
    }
}
