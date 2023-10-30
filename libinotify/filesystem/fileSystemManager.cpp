#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace inotify {
    class fileSystemManager {
    public:
        std::fstream openFile(const std::string& path) {
            std::fstream file;
            file.open(path, std::ios::in | std::ios::out);
            return file;
        }

        std::vector<std::filesystem::path> checkAllFiles(const std::filesystem::path& path) {
            std::vector<std::filesystem::path> files;
            for(auto& p: std::filesystem::recursive_directory_iterator(path)) {
                files.push_back(p.path());
            }
            return files;
        }

        bool isFileOrFolder(const std::filesystem::path& path) {
            if (std::filesystem::is_regular_file(path)) {
                std::cout << path << " is a file" << std::endl;
                return true;
            }
            else if (std::filesystem::is_directory(path)) {
                std::cout << path << " is a directory" << std::endl;
                return false;
            }
            else {
                std::cout << path << " is not a file nor a directory" << std::endl;
                return false;
            }
        }
    };
}

