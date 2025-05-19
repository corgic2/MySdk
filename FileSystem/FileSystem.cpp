#include "FileSystem.h"
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
namespace my_sdk
{
    FileSystem::FileSystem()
    {
    }

    FileSystem::~FileSystem()
    {
    }

    bool FileSystem::WriteStringToFile(const boost::filesystem::path& filePath, const std::string& str, bool writeBom)
    {
        // 确保父目录存在
        boost::filesystem::create_directories(filePath.parent_path());

        // 写入内容
        std::ofstream file(filePath.string(), std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }
        // 可选写入BOM
        if (writeBom)
        {
            const char bom[] = "\xEF\xBB\xBF";
            file.write(bom, 3);
        }
        file << str;
        file.close();
        std::cout << "文件已写入: " << filePath << "\n";
        return true;
    }

    std::string FileSystem::ReadStringFromFile(const boost::filesystem::path& filePath, bool removeBOM)
    {
        if (!boost::filesystem::exists(filePath))
        {
            return "";
        }

        std::ifstream file(filePath.string(), std::ios::binary);
        if (!file.is_open())
        {
            return "";
        }
        // 读取全部内容到缓冲区
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        // 安全移除BOM
        if (removeBOM && content.size() >= 3 && static_cast<uint8_t>(content[0]) == 0xEF && static_cast<uint8_t>(content[1]) == 0xBB && static_cast<uint8_t>(content[2]) == 0xBF)
        {
            content.erase(0, 3);
        }

        return content;
    }

    std::string FileSystem::ConvertEncodingToUtf_8(const std::string& input, const std::string& to)
    {
        return input;
    }

    void FileSystem::ListDirectory(const boost::filesystem::path& dir)
    {
        if (!boost::filesystem::exists(dir) || !boost::filesystem::is_directory(dir))
        {
            throw std::runtime_error("目录无效: " + dir.string());
        }

        std::cout << "目录内容 (" << dir << "):\n";
        for (const auto& entry : boost::filesystem::directory_iterator(dir))
        {
            const auto& path = entry.path();
            std::string type = boost::filesystem::is_directory(path) ? "[目录]" : boost::filesystem::is_regular_file(path) ? "[文件]" : "[其他]";
            std::cout << "  " << type << " " << path.filename() << "\n";
        }
    }

    std::vector<boost::filesystem::path> FileSystem::Getfiles(const boost::filesystem::path& dir)
    {
        std::vector<boost::filesystem::path> files;
        if (!boost::filesystem::is_directory(dir))
        {
            return files;
        }

        for (const auto& entry : boost::filesystem::directory_iterator(dir))
        {
            if (boost::filesystem::is_regular_file(entry))
            {
                files.push_back(entry.path());
            }
        }

        return files;
    }
}
