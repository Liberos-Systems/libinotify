#pragma once
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

#include <sys/inotify.h>
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include "filesystem/file_system.hpp"

#include "fmt/fmt.hpp"

struct Timestamp {
    std::chrono::system_clock::time_point time; // Time of the event occurrence
};

struct File {
    std::filesystem::path file; // File that triggered the event
};

struct Event {
    std::string event; // The event that occurred
};


namespace inotify
{
    class Watcher
    {
    private:
        FileSystem file_system_ = null;
        
        std::vector<std::filesystem::path> watch_list_;
        std::atomic<bool> run_watcher_thread_;
        std::map<std::filesystem::path, std::queue<std::string>> file_events_; // Map where the first element is the path, and the second is a queue of events
        std::thread observer_thread_;
        std::function<void()> stored_function_;                                // In this field is stored function to call at anyevent
        
        bool verbose_;                                                         // Add verbose flag
        bool recursive_mode_ = false;
        bool running_ = false;
        int fd_;                                                               // file descriptor for inotify

        void observeFiles();



    public:
        Watcher();
        void enable()
        {
            this->run_watcher_thread_ = true;
            spdlog::info("Watcher enabled");
        }
        nlohmann::json watch() const;
        template <typename Callable>
        void call(Callable &&func) // Changed return type to void
        {
            stored_function_ = func; // Store function instead of calling it
        }
        void disable()
        {
            this->run_watcher_thread_ = false;
            spdlog::info("Watcher disabled");
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

        //syf
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
    };
}

// void callcall() // Added method to call stored function
//         {
//             if(stored_function_) // Check if function is stored
//             {
//                 stored_function_(); // Call stored function
//             }
//         }