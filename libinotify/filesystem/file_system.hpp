#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <spdlog/spdlog.h>

namespace inotify
{
    class FileSystemManager
    {
    private:
        std::filesystem::path root_;
        std::vector<std::filesystem::path> absolutePaths_;

    public:
        FileSystemManager() : root_(std::filesystem::current_path()) {}
        std::vector<std::string> ls(const std::string &path = "")
        {
            std::filesystem::path targetPath;
            std::vector<std::string> filePaths;

            if (path == "~")
            {
                std::filesystem::path homeDir = std::filesystem::path(std::getenv("HOME"));
                if (homeDir.empty())
                {
                    spdlog::error("Failed to get home directory.");
                    return filePaths;
                }
                targetPath = homeDir;
            }
            else if (path.empty() || path == "*")
            {
                targetPath = root_;
            }
            else
            {
                std::filesystem::path tempPath(path);
                if (tempPath.is_relative())
                {
                    targetPath = root_ / tempPath;
                }
                else
                {
                    targetPath = tempPath;
                }
            }

            if (!std::filesystem::is_directory(targetPath))
            {
                spdlog::error("Provided path is not a directory: {}", targetPath.string());
                return filePaths;
            }

            for (const auto &entry : std::filesystem::directory_iterator(targetPath))
            {
                if (entry.is_directory())
                {
                    for (const auto &subEntry : std::filesystem::directory_iterator(entry.path()))
                    {
                        filePaths.push_back(subEntry.path().string());
                    }
                }
                else
                {
                    filePaths.push_back(entry.path().string());
                }
            }

            return filePaths;
        }
        bool cp(const std::string &src, const std::string &dst)
        {
            std::filesystem::path srcPath(src);
            std::filesystem::path dstPath(dst);
            if (std::filesystem::exists(srcPath))
            {
                if (!std::filesystem::exists(dstPath.parent_path()))
                {
                    try
                    {
                        std::filesystem::create_directories(dstPath.parent_path());
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("Failed to create directory: {}", dstPath.parent_path().string());
                        return false;
                    }
                }
                try
                {
                    std::filesystem::copy(srcPath, dstPath);
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Failed to copy file: {}", src);
                    return false;
                }
                return true;
            }
            else
            {
                spdlog::error("Source file does not exist: {}", src);
                return false;
            }
        }

        bool mv(const std::string &src, const std::string &dst)
        {
            std::filesystem::path srcPath(src);
            std::filesystem::path dstPath(dst);
            if (std::filesystem::exists(srcPath))
            {
                if (!std::filesystem::exists(dstPath.parent_path()))
                {
                    try
                    {
                        std::filesystem::create_directories(dstPath.parent_path());
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("Failed to create directory: {}", dstPath.parent_path().string());
                        return false;
                    }
                }
                try
                {
                    std::filesystem::rename(srcPath, dstPath);
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Failed to move file: {}", src);
                    return false;
                }
                return true;
            }
            else
            {
                spdlog::error("Source file does not exist: {}", src);
                return false;
            }
        }
        bool rm(const std::string &name)
        {
            std::filesystem::path _path(name);
            if (std::filesystem::exists(_path))
            {
                try
                {
                    std::filesystem::remove(_path);
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Failed to remove file: {}", name);
                    return false;
                }
                return true;
            }
            else
            {
                spdlog::error("File does not exist: {}", name);
                return false;
            }
        }

        bool rmdir(const std::string &name)
        {
            std::filesystem::path _path(name);
            if (std::filesystem::exists(_path) && std::filesystem::is_directory(_path))
            {
                try
                {
                    std::filesystem::remove_all(_path);
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Failed to remove directory: {}", name);
                    return false;
                }
                return true;
            }
            else
            {
                spdlog::error("Directory does not exist: {}", name);
                return false;
            }
        }
        bool mkdir(const std::string &name)
        {
            if (name.back() == '/')
            {
                spdlog::error("Path ends with '/': {}", name);
                return false;
            }

            std::string _name = name;
            std::filesystem::path _path;
            if (_name.rfind("./", 0) == 0) // If name starts with "./"
            {
                spdlog::warn("Converted relative path to absolute");
                try
                {
                    _name.erase(0, 1);
                    std::filesystem::path path(_name);
                    path = root_ / path.relative_path();
                    path = path.relative_path();
                    _path = path;
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Failed to create path. Error: {}", e.what());
                    return false;
                }
            }
            else
            {
                try
                {
                    std::filesystem::path path(_name);
                    _path = path;
                }
                catch (const std::exception &e)
                {
                    spdlog::error("Failed to create path. Error: {}", e.what());
                    return false;
                }
            }

            if (_path.generic_string().find('/') != std::string::npos)
            {
                std::filesystem::path newDir = _path;
                std::filesystem::path intermediatePath;
                for (auto &component : newDir)
                {
                    intermediatePath /= component;
                    try
                    {
                        if (!std::filesystem::exists(intermediatePath))
                        {
                            std::filesystem::create_directory(intermediatePath);
                            spdlog::info("Created directory: {}", intermediatePath.string());
                        }
                    }
                    catch (const std::exception &e)
                    {
                        spdlog::error("Failed to create directory: {}. Error: {}", intermediatePath, e.what());
                        return false;
                    }
                }
                return true;
            }
            else
            {
                spdlog::error("Path is not in Unix format: {}", _path.string());
                return false;
            }
        }
        bool touch(const std::string &name)
        {
            std::filesystem::path path(name);
            if (!path.is_absolute())
            {
                path = root_ / path;
            }
            if (path.generic_string().find('/') != std::string::npos)
            {
                std::filesystem::path newFile = path.parent_path() / path.filename();
                if (!std::filesystem::exists(newFile))
                {
                    std::ofstream file(newFile);
                    return true;
                }
                else
                {
                    spdlog::error("File already exists: {}", newFile);
                    return false;
                }
            }
            else
            {
                spdlog::error("Path is not in Unix format: {}", path);
                return false;
            }
        }
        std::filesystem::path pwd() const { return root_; }
        bool cd(const std::filesystem::path &path)
        {
            if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
            {
                root_ = path;
                return true;
            }
            else
            {
                spdlog::error("Path does not exist or is not a directory: {}", path);
                return false;
            }
        }
    };
}


