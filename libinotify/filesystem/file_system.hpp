#include <filesystem>
#include <vector>
#include <string>

namespace inotify
{
    class FileSystemManager
    {
    private:
        std::vector<std::string> absolutePaths;

    public:
        void addFile(const std::string &path);
        void addPath(const std::filesystem::path &path);
        void addFolder(const std::filesystem::path &path);
        void addFolder(const std::filesystem::path &path, bool recursive);
        void tree();
    };
}
