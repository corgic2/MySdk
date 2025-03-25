#include "FileSystem.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <windows.h>

namespace my_sdk
{
    FileSystem::FileSystem()
    {
    }

    FileSystem::~FileSystem()
    {
    }

    std::string FileSystemUtils::CombinePath(const std::string& path1, const std::string& path2)
    {
        // 创建 path1 和 path2 的拷贝，以避免修改原始输入
        std::string path1Copy = path1;
        std::string path2Copy = path2;
        // 替换正斜杠为反斜杠
        std::replace(path1Copy.begin(), path1Copy.end(), '/', '\\');
        std::replace(path2Copy.begin(), path2Copy.end(), '/', '\\');
        // 拼接路径
        return path1Copy + "\\" + path2Copy;
    }

    bool FileSystem::WriteStringToFile(const std::string& filePath, const std::string& fileName, const std::string& str)
    {
        //采用二进制读写确保数据没有问题 + 优化读取速率
        std::ofstream file(FileSystemUtils::CombinePath(filePath, fileName), std::ios::binary);
        // 文件流没有被打开

        if (!file.is_open())
        {
            return false;
        }

        // 写入
        const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
        file.write(reinterpret_cast<const char*>(bom), 3);
        file << str;
        file.close();
        return true;
    }

    void FileSystem::ReadStringFromFile(const std::string& filePath, const std::string& fileName, std::string& outData)
    {
        //采用二进制读写确保数据没有问题 + 优化读取速率
        std::ifstream file(FileSystemUtils::CombinePath(filePath, fileName), std::ios::binary);

        if (!file.is_open())
        {
            return;
        }

        // 检测BOM
        char bom[3];
        file.read(bom, 3);
        if (file.gcount() == 3 &&
            static_cast<unsigned char>(bom[0]) == 0xEF &&
            static_cast<unsigned char>(bom[1]) == 0xBB &&
            static_cast<unsigned char>(bom[2]) == 0xBF)
        {
            // 跳过BOM
        }
        else
        {
            file.seekg(0); // 无BOM则重置指针
        }

        outData.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
    }
}