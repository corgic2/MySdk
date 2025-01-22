#pragma once
#include <iostream>
#include <string>
#include "FileSystem.h"

void FileSystemTest()
{
    // 测试路径和文件名
    std::string testDir = "E:/lf_documents/lf_project/E__WorkSpace/MyQtWorkSpace/sdk";  // 测试目录
    std::string testFile = "test.txt";  // 测试文件名
    std::string testContent = "Hello, FileSystem!";  // 测试内容
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
