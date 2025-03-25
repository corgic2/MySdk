#pragma once
#include <iostream>
#include <string>
#include "FileSystem.h"
using namespace my_sdk;

void FileSystemTest()
{
    // 测试路径和文件名
    std::string testDir = "../TestFiles";           // 测试目录
    std::string testFile = "testForFileSystem.txt"; // 测试文件名
    std::string testContent = "你好，我的测试SDK文件系统";     // 测试内容
    // 1. 创建测试目录
    std::string testPath = FileSystemUtils::CombinePath(testDir, testFile);
    std::cout << "File created successfully." << std::endl;
    // 3. 写入文件
    std::cout << "Testing: Write to File" << std::endl;

    if (!FileSystem::WriteStringToFile(testDir, testFile, testContent))
    {
        std::cerr << "Failed to write to file: " << testFile << std::endl;
        return;
    }

    std::cout << "File written successfully." << std::endl;
    // 4. 读取文件
    std::cout << "Testing: Read from File" << std::endl;
    std::string readContent;
    FileSystem::ReadStringFromFile(testDir, testFile, readContent);

    if (readContent != testContent)
    {
        std::cerr << "File content mismatch. Expected: " << testContent << ", Got: " << readContent << std::endl;
        return;
    }

    std::cout << "File read successfully. Content: " << readContent << std::endl;
}
