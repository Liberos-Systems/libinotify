
#include "spdlog/spdlog.h"

namespace inotify
{
    class FileSystemManager
    {
    private:
        std::vector<std::filesystem::path> absolutePaths;

    public:
        void addFile(const std::filesystem::path &path)
        {
            if (std::filesystem::exists(path))
            {
                std::filesystem::path absPath = std::filesystem::absolute(path);
                absolutePaths.push_back(absPath);

                std::fstream file;
                try
                {
                    file.open(absPath, std::ios::out);
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Failed to open file: {}. Error: {}", absPath, e.what());
                }
            }
            else
            {
                spdlog::error("File does not exist: {}", path);
            }
        }

        void addPath(const std::filesystem::path &path)
        {
            if (std::filesystem::exists(path))
            {
                if (std::filesystem::is_directory(path))
                {
                    try
                    {
                        for (auto &p : std::filesystem::directory_iterator(path))
                        {
                            if (std::filesystem::is_directory(p))
                            {
                                addPath(p.path());
                            }
                            else if (std::filesystem::is_regular_file(p))
                            {
                                addFile(p.path());
                            }
                        }
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("Failed to iterate over directory: {}. Error: {}", path, e.what());
                    }
                }
                else if (std::filesystem::is_regular_file(path))
                {
                    addFile(path);
                }
            }
            else
            {
                spdlog::error("Path does not exist: {}", path);
            }
        }
        
        void addFolder(const std::filesystem::path &path)
        {
            if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
            {
                try
                {
                    for (auto &p : std::filesystem::directory_iterator(path))
                    {
                        if (std::filesystem::is_regular_file(p))
                        {
                            addFile(p.path());
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
                spdlog::error("Directory does not exist or is not a directory: {}", path);
            }
        }

        void addFolder(const std::filesystem::path &path, bool recursive)
        {
            if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
            {
                if (recursive)
                {
                    try
                    {
                        for (auto &p : std::filesystem::recursive_directory_iterator(path))
                        {
                            if (std::filesystem::is_regular_file(p))
                            {
                                addFile(p.path());
                            }
                        }
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("Failed to recursively iterate over directory: {}. Error: {}", path, e.what());
                    }
                }
                else
                {
                    addFolder(path);
                }
            }
            else
            {
                spdlog::error("Directory does not exist or is not a directory: {}", path);
            }
        }

        void tree()
        {
            spdlog::info("Loaded files and directories:");
            for (const auto &path : absolutePaths)
            {
                spdlog::info("{}", path);
            }
        }
    };
}