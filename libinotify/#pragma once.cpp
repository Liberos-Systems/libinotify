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