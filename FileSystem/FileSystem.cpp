#include "FileSystem.h"
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <sys/stat.h>
#include <ctime>

namespace my_sdk
{
    FileSystem::FileSystem()
    {
    }

    FileSystem::~FileSystem()
    {
    }

    bool FileSystem::WriteStringToFile(const std::string& filePath, const std::string& str, bool writeBom)
    {
        try
        {
            // 确保父目录存在
            boost::filesystem::path path(filePath);
            boost::filesystem::create_directories(path.parent_path());

            // 写入内容
            std::ofstream file(filePath, std::ios::binary);
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
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    std::string FileSystem::ReadStringFromFile(const std::string& filePath, bool removeBOM)
    {
        try
        {
            if (!boost::filesystem::exists(filePath))
            {
                return "";
            }

            std::ifstream file(filePath, std::ios::binary);
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
        catch (const std::exception&)
        {
            return "";
        }
    }

    ST_FileInfo FileSystem::GetFileInfo(const std::string& path)
    {
        ST_FileInfo info;
        try
        {
            boost::filesystem::path fsPath(path);
            if (!boost::filesystem::exists(fsPath))
            {
                return info;
            }

            info.m_name = fsPath.filename().string();
            info.m_path = fsPath.string();
            info.m_isDirectory = boost::filesystem::is_directory(fsPath);

            boost::filesystem::file_status status = boost::filesystem::status(fsPath);
            info.m_isReadOnly = (status.permissions() & boost::filesystem::perms::owner_write) == boost::filesystem::perms::no_perms;

            if (!info.m_isDirectory)
            {
                info.m_size = boost::filesystem::file_size(fsPath);
            }

            std::time_t ctime = boost::filesystem::last_write_time(fsPath);
            info.m_createTime = ctime;
            info.m_modifyTime = ctime;
            info.m_accessTime = ctime;
        }
        catch (const std::exception&)
        {
        }
        return info;
    }

    bool FileSystem::Copy(const std::string& source, const std::string& destination, bool overwrite)
    {
        try
        {
            boost::filesystem::path srcPath(source);
            boost::filesystem::path destPath(destination);

            if (!boost::filesystem::exists(srcPath))
            {
                return false;
            }

            if (boost::filesystem::exists(destPath) && !overwrite)
            {
                return false;
            }

            if (boost::filesystem::is_directory(srcPath))
            {
                boost::filesystem::create_directories(destPath);
                for (boost::filesystem::directory_iterator file(srcPath); file != boost::filesystem::directory_iterator(); ++file)
                {
                    boost::filesystem::path current(file->path());
                    boost::filesystem::path target = destPath / current.filename();
                    Copy(current.string(), target.string(), overwrite);
                }
            }
            else
            {
                if (overwrite && boost::filesystem::exists(destPath))
                {
                    boost::filesystem::remove(destPath);
                }
                boost::filesystem::copy_file(srcPath, destPath);
            }
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool FileSystem::Move(const std::string& source, const std::string& destination)
    {
        try
        {
            boost::filesystem::path srcPath(source);
            boost::filesystem::path destPath(destination);
            boost::filesystem::rename(srcPath, destPath);
            return true;
        }
        catch (const std::exception&)
        {
            // 如果跨设备移动失败，尝试复制后删除
            if (Copy(source, destination, true))
            {
                return Delete(source);
            }
            return false;
        }
    }

    bool FileSystem::Delete(const std::string& path)
    {
        try
        {
            boost::filesystem::path fsPath(path);
            if (!boost::filesystem::exists(fsPath))
            {
                return true;
            }
            boost::filesystem::remove_all(fsPath);
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool FileSystem::CreateDirectory(const std::string& path)
    {
        try
        {
            return boost::filesystem::create_directories(path);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool FileSystem::Exists(const std::string& path)
    {
        try
        {
            return boost::filesystem::exists(path);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    size_t FileSystem::GetFileSize(const std::string& path)
    {
        try
        {
            boost::filesystem::path fsPath(path);
            if (boost::filesystem::exists(fsPath) && !boost::filesystem::is_directory(fsPath))
            {
                return boost::filesystem::file_size(fsPath);
            }
        }
        catch (const std::exception&)
        {
        }
        return 0;
    }

    std::vector<std::string> FileSystem::GetFiles(const std::string& dir, bool recursive)
    {
        std::vector<std::string> files;
        std::vector<std::string> directories;

        try
        {
            if (recursive)
            {
                GetDirectoryContents(dir, files, directories);
            }
            else
            {
                boost::filesystem::path dirPath(dir);
                if (!boost::filesystem::exists(dirPath) || !boost::filesystem::is_directory(dirPath))
                {
                    return files;
                }

                for (boost::filesystem::directory_iterator it(dirPath); it != boost::filesystem::directory_iterator(); ++it)
                {
                    if (boost::filesystem::is_regular_file(it->path()))
                    {
                        files.push_back(it->path().string());
                    }
                }
            }
        }
        catch (const std::exception&)
        {
        }
        return files;
    }

    std::vector<std::string> FileSystem::GetDirectories(const std::string& dir, bool recursive)
    {
        std::vector<std::string> files;
        std::vector<std::string> directories;

        try
        {
            if (recursive)
            {
                GetDirectoryContents(dir, files, directories);
            }
            else
            {
                boost::filesystem::path dirPath(dir);
                if (!boost::filesystem::exists(dirPath) || !boost::filesystem::is_directory(dirPath))
                {
                    return directories;
                }

                for (boost::filesystem::directory_iterator it(dirPath); it != boost::filesystem::directory_iterator(); ++it)
                {
                    if (boost::filesystem::is_directory(it->path()))
                    {
                        directories.push_back(it->path().string());
                    }
                }
            }
        }
        catch (const std::exception&)
        {
        }
        return directories;
    }

    bool FileSystem::SetFileAttributes(const std::string& path, bool readOnly)
    {
        try
        {
            boost::filesystem::path fsPath(path);
            if (!boost::filesystem::exists(fsPath))
            {
                return false;
            }

            boost::filesystem::permissions(fsPath, 
                readOnly ? boost::filesystem::perms::owner_read | boost::filesystem::perms::group_read | boost::filesystem::perms::others_read
                        : boost::filesystem::perms::owner_read | boost::filesystem::perms::owner_write |
                          boost::filesystem::perms::group_read | boost::filesystem::perms::group_write |
                          boost::filesystem::perms::others_read | boost::filesystem::perms::others_write);
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    std::string FileSystem::GetExtension(const std::string& path)
    {
        try
        {
            return boost::filesystem::path(path).extension().string();
        }
        catch (const std::exception&)
        {
            return "";
        }
    }

    std::string FileSystem::GetFileNameWithoutExtension(const std::string& path)
    {
        try
        {
            return boost::filesystem::path(path).stem().string();
        }
        catch (const std::exception&)
        {
            return "";
        }
    }

    void FileSystem::GetDirectoryContents(const std::string& dir,
                                        std::vector<std::string>& files,
                                        std::vector<std::string>& directories)
    {
        try
        {
            boost::filesystem::path dirPath(dir);
            if (!boost::filesystem::exists(dirPath) || !boost::filesystem::is_directory(dirPath))
            {
                return;
            }

            for (boost::filesystem::recursive_directory_iterator it(dirPath); it != boost::filesystem::recursive_directory_iterator(); ++it)
            {
                if (boost::filesystem::is_regular_file(it->path()))
                {
                    files.push_back(it->path().string());
                }
                else if (boost::filesystem::is_directory(it->path()))
                {
                    directories.push_back(it->path().string());
                }
            }
        }
        catch (const std::exception&)
        {
        }
    }

    void FileSystem::ListDirectory(const std::string& dir)
    {
        try
        {
            boost::filesystem::path dirPath(dir);
            if (!boost::filesystem::exists(dirPath) || !boost::filesystem::is_directory(dirPath))
            {
                throw std::runtime_error("目录无效: " + dir);
            }

            std::cout << "目录内容 (" << dir << "):\n";
            for (const auto& entry : boost::filesystem::directory_iterator(dirPath))
            {
                const auto& path = entry.path();
                std::string type = boost::filesystem::is_directory(path) ? "[目录]" : boost::filesystem::is_regular_file(path) ? "[文件]" : "[其他]";
                std::cout << "  " << type << " " << path.filename() << "\n";
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "列出目录内容失败: " << e.what() << std::endl;
        }
    }

    std::string FileSystem::ConvertEncodingToUtf_8(const std::string& input, const std::string& to)
    {
        // TODO: 实现编码转换功能
        return input;
    }
}
