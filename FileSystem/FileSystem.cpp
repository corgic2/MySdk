#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <windows.h>
#include "FileSystem.h"
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
    std::ofstream file(FileSystemUtils::CombinePath(filePath, fileName));
    // 文件流没有被打开

    if (!file.is_open())
    {
        return false;
    }

    // 写入
    file << str;
    file.close();
    return true;
}

void FileSystem::ReadStringFromFile(const std::string& filePath, const std::string& fileName, std::string& outData)
{
    std::ifstream file(FileSystemUtils::CombinePath(filePath, fileName));

    if (!file.is_open())
    {
        return;
    }

    outData.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
}
}