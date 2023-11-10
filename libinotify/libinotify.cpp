// libinotify.cpp
// compile with: cl /c /EHsc libinotify.cpp
// post-build command: lib libinotify.obj

#include "libinotify.hpp"

namespace inotify
{
    void Watcher::excludeFile(const std::string& file)
    {
        // Implementation of excluding a file from being watched
    }

    void Watcher::fromFile(const std::string& file)
    {
        // Implementation of reading filenames to watch or exclude from a file
    }

    void Watcher::zero()
    {
        // Implementation of outputting table rows and columns even if all elements are zero
    }

    void Watcher::exclude(const std::string& pattern)
    {
        // Implementation of not processing any events whose filename matches the specified POSIX extended regular expression, case sensitive
    }

    void Watcher::excludei(const std::string& pattern)
    {
        // Implementation of not processing any events whose filename matches the specified POSIX extended regular expression, case insensitive
    }

    void Watcher::recursive()
    {
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
