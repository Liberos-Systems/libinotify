#pragma once
#include <sys/inotify.h>
#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <chrono>
#include <thread>
#include <queue>
#include <map>
#include <atomic>
#include <functional>
#include "filesystem/file_system.hpp"

namespace inotify
{
    class Watcher
    {
    private:
        bool verbose_; // Add verbose flag
        std::vector<std::filesystem::path> watch_list_;
        std::atomic<bool> run_watcher_thread_;
        std::map<std::filesystem::path, std::queue<std::string>> file_events_; // Map where the first element is the path, and the second is a queue of events
        std::thread observer_thread_;
        bool recursive_mode_ = false;
        bool running_ = false;
        void observeFiles();
        int fd_;
        std::function<void()> stored_function_; // In this field is stored function to call at anyevent
        FileSystemManager file_system_;

    public:
        Watcher();
        void enable()
        {
            this->run_watcher_thread_ = true;
            spdlog::info("Watcher enabled");
        }

        void excludeFile(const std::string &file);
        void fromFile(const std::string &file);
        void zero();
        void exclude(const std::string &pattern);
        void excludei(const std::string &pattern);
        void recursive(const std::string &path);
        void timeout(int seconds);
        void event(const std::string &event);
        void ascending(const std::string &event);
        void descending(const std::string &event);
        bool setVerbose(bool value);
        bool getVerbose() const;
        nlohmann::json watch() const;
        void disable()
        {
            this->run_watcher_thread_ = false;
            spdlog::info("Watcher disabled");
        }
        template <typename Callable>
        void call(Callable &&func) // Changed return type to void
        {
            stored_function_ = func; // Store function instead of calling it
        }
        ~Watcher()
        {
            spdlog::warn("Object has been deleted"); // Log warning that the object has been deleted
            spdlog::shutdown();                      // Stop logging
            run_watcher_thread_ = false;
            if (observer_thread_.joinable())
            {
                observer_thread_.join();
            }
        }
    };
}

// void callcall() // Added method to call stored function
//         {
//             if(stored_function_) // Check if function is stored
//             {
//                 stored_function_(); // Call stored function
//             }
//         }




std::vector<std::filesystem::path> getAbsolutePaths()
        {
            return absolutePaths_;
        }
        std::vector<std::filesystem::path> getRelativePaths()
        {
            std::vector<std::filesystem::path> relativePaths;
            for (const auto &path : absolutePaths_)
            {
                relativePaths.push_back(root_.relative_path());
            }
            return relativePaths;
        }
        void tree()
        {
            for (const auto &path : absolutePaths_)
            {
                std::cout << path << std::endl;
                for (auto &p : std::filesystem::recursive_directory_iterator(path))
                {
                    std::cout << "  " << p.path() << std::endl;
                }
            }
        }

        void addFile(const std::filesystem::path &path)
        {
            if (std::filesystem::is_regular_file(path))
            {
                std::filesystem::path absPath = std::filesystem::absolute(path);
                auto it = std::find(absolutePaths_.begin(), absolutePaths_.end(), absPath);
                if (it != absolutePaths_.end())
                {
                    int count = std::count(absolutePaths_.begin(), absolutePaths_.end(), absPath);
                    spdlog::warn("File path already exists in the list. Total count: {}", count);
                }
                absolutePaths_.push_back(absPath);
            }
            else
            {
                spdlog::error("Path is not a regular file: {}", path);
            }
        }
        void addFolder(const std::filesystem::path &path, bool recursive = false)
        {
            if (std::filesystem::is_directory(path))
            {
                std::filesystem::path absPath = std::filesystem::absolute(path);
                auto it = std::find(absolutePaths_.begin(), absolutePaths_.end(), absPath);
                if (it != absolutePaths_.end())
                {
                    int count = std::count(absolutePaths_.begin(), absolutePaths_.end(), absPath);
                    spdlog::warn("Folder path already exists in the list. Total count: {}", count);
                }
                absolutePaths_.push_back(absPath);
                if (recursive)
                {
                    for (auto &p : std::filesystem::recursive_directory_iterator(path))
                    {
                        if (std::filesystem::is_regular_file(p))
                        {
                            addFile(p.path());
                        }
                        else if (std::filesystem::is_directory(p))
                        {
                            addFolder(p.path(), true);
                        }

                    }
                }
            }
            else
            {
                spdlog::error("Path is not a directory: {}", path);
            }
        }
        void addPath(const std::filesystem::path &path, int depth = 0)
        {
            if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
            {
                try
                {
                    if (depth != 0)
                    {
                        for (auto &p : std::filesystem::recursive_directory_iterator(path))
                        {
                            if (std::filesystem::is_regular_file(p))
                            {
                                addFile(p.path());
                            }
                            if (--depth == 0) break;
                        }
                    }
                    else
                    {
                        for (auto &p : std::filesystem::directory_iterator(path))
                        {
                            if (std::filesystem::is_regular_file(p))
                            {
                                addFile(p.path());
                            }
                        }
                    }
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Failed to iterate over directory: {}. Error: {}", path, e.what());
                }
            }
            else
            {
                spdlog::error("Path does not exist or is not a directory: {}", path);
            }
        }
        void removeFile(const std::filesystem::path &path)
        {
            std::filesystem::path absPath = std::filesystem::absolute(path);
            auto it = std::find(absolutePaths_.begin(), absolutePaths_.end(), absPath);
            if (it != absolutePaths_.end())
            {
                absolutePaths_.erase(it);
                int count = std::count(absolutePaths_.begin(), absolutePaths_.end(), absPath);
                if (count > 0)
                {
                    spdlog::warn("There are still {} instances of the file path in the list.", count);
                }
            }
            else
            {
                spdlog::error("File path does not exist in the list: {}", path);
            }
        }
        void removeFolder(const std::filesystem::path &path, int depth = 0, bool recursive = false)
        {
            std::filesystem::path absPath = std::filesystem::absolute(path);
            auto it = std::find(absolutePaths_.begin(), absolutePaths_.end(), absPath);
            if (it != absolutePaths_.end())
            {
                if (recursive)
                {
                    for (auto &p : std::filesystem::recursive_directory_iterator(path))
                    {
                        if (std::filesystem::is_regular_file(p))
                        {
                            removeFile(p.path());
                        }
                        else if (std::filesystem::is_directory(p))
                        {
                            removeFolder(p.path(), depth, true);
                        }
                    }
                }
                else if (depth != 0)
                {
                    for (auto &p : std::filesystem::directory_iterator(path))
                    {
                        if (std::filesystem::is_regular_file(p))
                        {
                            removeFile(p.path());
                        }
                        if (--depth == 0) break;
                    }
                }
                else
                {
                    for (auto &p : std::filesystem::directory_iterator(path))
                    {
                        if (std::filesystem::is_regular_file(p))
                        {
                            removeFile(p.path());
                        }
                    }
                }
                absolutePaths_.erase(it);
                int count = std::count(absolutePaths_.begin(), absolutePaths_.end(), absPath);
                if (count > 0)
                {
                    spdlog::warn("There are still {} instances of the folder path in the list.", count);
                }
            }
            else
            {
                spdlog::warn("Folder path does not exist in the list: {}", path);
            }
        }



        #pragma once
#include <sys/inotify.h>
#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <chrono>
#include <thread>
#include <queue>
#include <map>
#include <atomic>
#include <functional>
#include "filesystem/file_system.hpp"

namespace inotify
{
    class Watcher
    {
    private:
        bool verbose_; // Flag for verbose mode
        std::vector<std::filesystem::path> watch_list_; // List of files to watch
        std::atomic<bool> run_watcher_thread_; // Atomic boolean to control the watcher thread
        std::map<std::filesystem::path, std::queue<std::string>> file_events_; // Map to store file events, with the file path as key and a queue of events as value
        std::thread observer_thread_; // Thread to observe the files
        bool recursive_mode_ = false; // Flag for recursive mode
        bool running_ = false; // Flag to check if the watcher is running
        int fd_; // File descriptor for inotify
        std::function<void()> stored_function_; // Function to be called at any event
        FileSystemManager file_system_; // Instance of FileSystemManager to manage file system operations

        void observeFiles()
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

    public:
        Watcher()
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

            fd_ = inotify_init();
            if (fd_ < 0)
            {
                spdlog::error("Failed to initialize inotify.");
                throw std::runtime_error("Failed to initialize inotify.");
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

        std::vector<std::filesystem::path> getAbsolutePaths()
        {
            return absolutePaths_;
        }
        std::vector<std::filesystem::path> getRelativePaths()
        {
            std::vector<std::filesystem::path> relativePaths;
            for (const auto &path : absolutePaths_)
            {
                relativePaths.push_back(root_.relative_path());
            }
            return relativePaths;
        }
        void tree()
        {
            for (const auto &path : absolutePaths_)
            {
                std::cout << path << std::endl;
                for (auto &p : std::filesystem::recursive_directory_iterator(path))
                {
                    std::cout << "  " << p.path() << std::endl;
                }
            }
        }

        void addFile(const std::filesystem::path &path)
        {
            if (std::filesystem::is_regular_file(path))
            {
                std::filesystem::path absPath = std::filesystem::absolute(path);
                auto it = std::find(absolutePaths_.begin(), absolutePaths_.end(), absPath);
                if (it != absolutePaths_.end())
                {
                    int count = std::count(absolutePaths_.begin(), absolutePaths_.end(), absPath);
                    spdlog::warn("File path already exists in the list. Total count: {}", count);
                }
                absolutePaths_.push_back(absPath);
            }
            else
            {
                spdlog::error("Path is not a regular file: {}", path);
            }
        }
        void addFolder(const std::filesystem::path &path, bool recursive = false)
        {
            if (std::filesystem::is_directory(path))
            {
                std::filesystem::path absPath = std::filesystem::absolute(path);
                auto it = std::find(absolutePaths_.begin(), absolutePaths_.end(), absPath);
                if (it != absolutePaths_.end())
                {
                    int count = std::count(absolutePaths_.begin(), absolutePaths_.end(), absPath);
                    spdlog::warn("Folder path already exists in the list. Total count: {}", count);
                }
                absolutePaths_.push_back(absPath);
                if (recursive)
                {
                    for (auto &p : std::filesystem::recursive_directory_iterator(path))
                    {
                        if (std::filesystem::is_regular_file(p))
                        {
                            addFile(p.path());
                        }
                        else if (std::filesystem::is_directory(p))
                        {
                            addFolder(p.path(), true);
                        }

                    }
                }
            }
            else
            {
                spdlog::error("Path is not a directory: {}", path);
            }
        }
        void addPath(const std::filesystem::path &path, int depth = 0)
        {
            if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
            {
                try
                {
                    if (depth != 0)
                    {
                        for (auto &p : std::filesystem::recursive_directory_iterator(path))
                        {
                            if (std::filesystem::is_regular_file(p))
                            {
                                addFile(p.path());
                            }
                            if (--depth == 0) break;
                        }
                    }
                    else
                    {
                        for (auto &p : std::filesystem::directory_iterator(path))
                        {
                            if (std::filesystem::is_regular_file(p))
                            {
                                addFile(p.path());
                            }
                        }
                    }
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Failed to iterate over directory: {}. Error: {}", path, e.what());
                }
            }
            else
            {
                spdlog::error("Path does not exist or is not a directory: {}", path);
            }
        }
        void removeFile(const std::filesystem::path &path)
        {
            std::filesystem::path absPath = std::filesystem::absolute(path);
            auto it = std::find(absolutePaths_.begin(), absolutePaths_.end(), absPath);
            if (it != absolutePaths_.end())
            {
                absolutePaths_.erase(it);
                int count = std::count(absolutePaths_.begin(), absolutePaths_.end(), absPath);
                if (count > 0)
                {
                    spdlog::warn("There are still {} instances of the file path in the list.", count);
                }
            }
            else
            {
                spdlog::error("File path does not exist in the list: {}", path);
            }
        }
        void removeFolder(const std::filesystem::path &path, int depth = 0, bool recursive = false)
        {
            std::filesystem::path absPath = std::filesystem::absolute(path);
            auto it = std::find(absolutePaths_.begin(), absolutePaths_.end(), absPath);
            if (it != absolutePaths_.end())
            {
                if (recursive)
                {
                    for (auto &p : std::filesystem::recursive_directory_iterator(path))
                    {
                        if (std::filesystem::is_regular_file(p))
                        {
                            removeFile(p.path());
                        }
                        else if (std::filesystem::is_directory(p))
                        {
                            removeFolder(p.path(), depth, true);
                        }
                    }
                }
                else if (depth != 0)
                {
                    for (auto &p : std::filesystem::directory_iterator(path))
                    {
                        if (std::filesystem::is_regular_file(p))
                        {
                            removeFile(p.path());
                        }
                        if (--depth == 0) break;
                    }
                }
                else
                {
                    for (auto &p : std::filesystem::directory_iterator(path))
                    {
                        if (std::filesystem::is_regular_file(p))
                        {
                            removeFile(p.path());
                        }
                    }
                }
                absolutePaths_.erase(it);
                int count = std::count(absolutePaths_.begin(), absolutePaths_.end(), absPath);
                if (count > 0)
                {
                    spdlog::warn("There are still {} instances of the folder path in the list.", count);
                }
            }
            else
            {
                spdlog::warn("Folder path does not exist in the list: {}", path);
            }
        }
    
        void enable()
        {
            this->run_watcher_thread_ = true;
            spdlog::info("Watcher enabled");
        }

        void excludeFile(const std::string &file)
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

        void fromFile(const std::string &file)
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
                        { // Show information if verbose is true
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
                        { // Show information if verbose is true
                            spdlog::info("Removed from watchlist: {}", line.substr(1));
                        }
                    }
                }
            }
        }

        void zero()
        {
        void excludeFile(const std::string &file);
        void fromFile(const std::string &file);
        void zero();
        void exclude(const std::string &pattern);
        void excludei(const std::string &pattern);
        void recursive(const std::string &path);
        void timeout(int seconds);
        void event(const std::string &event);
        void ascending(const std::string &event);
        void descending(const std::string &event);
        bool setVerbose(bool value);
        bool getVerbose() const;
        nlohmann::json watch() const;
        void disable()
        {
            this->run_watcher_thread_ = false;
            spdlog::info("Watcher disabled");
        }
        template <typename Callable>
        void call(Callable &&func) // Changed return type to void
        {
            stored_function_ = func; // Store function instead of calling it
        }
        ~Watcher()
        {
            spdlog::warn("Object has been deleted"); // Log warning that the object has been deleted
            spdlog::shutdown();                      // Stop logging
            run_watcher_thread_ = false;
            if (observer_thread_.joinable())
            {
                observer_thread_.join();
            }
        }
    };
}

// void callcall() // Added method to call stored function
//         {
//             if(stored_function_) // Check if function is stored
//             {
//                 stored_function_(); // Call stored function
//             }
//         }

#include "libinotify.hpp"
#include "fmt/fmt.hpp"

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

        fd_ = inotify_init();
        if (fd_ < 0)
        {
            spdlog::error("Failed to initialize inotify.");
            throw std::runtime_error("Failed to initialize inotify.");
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
                    { // Show information if verbose is true
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
                    { // Show information if verbose is true
                        spdlog::info("Removed from watchlist: {}", line.substr(1));
                    }
                }
            }
        }
    }

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

    nlohmann::json Watcher::watch() const
    {
        nlohmann::json emptyJson;
        emptyJson["date"] = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        return emptyJson;
    }
}



