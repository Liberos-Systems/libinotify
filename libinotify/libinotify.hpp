#pragma once
#include <sys/inotify.h>
#include "spdlog/spdlog.h"
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <chrono>
#include <thread>
#include <queue>
#include <map>

namespace inotify
{
    class Watcher
    {
    private:
        bool verbose; // Add verbose flag
        std::vector<std::filesystem::path> watchList;
        std::map<std::filesystem::path, std::queue<std::string>> fileEvents; // Map where the first element is the path, and the second is a queue of events
        std::thread observerThread;
        bool recursiveMode=false;
        void observeFiles();
        int fd;
    public:
        Watcher(); // Added constructor
        ~Watcher(); // Added destructor
        void excludeFile(const std::string& file);
        void fromFile(const std::string& file);
        void zero();
        void exclude(const std::string& pattern);
        void excludei(const std::string& pattern);
        void recursive(const std::string& path);
        void timeout(int seconds);
        void event(const std::string& event);
        void ascending(const std::string& event);
        void descending(const std::string& event);
        bool setVerbose(bool value);
        bool getVerbose() const;
        std::vector<std::string> getCurrentEvents() const; // Function to return current file events
    };
}