#pragma once
#include <iostream>
#include <string>
#include <QDir>
#include <QTextCodec>
#include "FileSystem.h"
using namespace my_sdk;

void FileSystemTest()
{
    try
    {
        // 设置默认编码为UTF-8
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

        // 测试路径和文件名
        std::string testDir = "../TestFiles";
        std::string testFile = testDir + "/testForFileSystem.txt";
        std::string testFile2 = testDir + "/testForFileSystem2.txt";
        std::string testContent = "你好，我的测试SDK文件系统";

        std::cout << "\n=== 文件系统测试开始 ===\n\n";

        // 1. 测试目录操作
        std::cout << "1. 测试目录操作\n";
        if (!FileSystem::Exists(testDir))
        {
            if (FileSystem::CreateDirectory(testDir))
            {
                std::cout << "创建目录成功: " << testDir << std::endl;
            }
        }

        // 2. 测试文件写入
        std::cout << "\n2. 测试文件写入\n";
        if (FileSystem::WriteStringToFile(testFile, testContent))
        {
            std::cout << "文件写入成功: " << testFile << std::endl;
        }

        // 3. 测试文件读取
        std::cout << "\n3. 测试文件读取\n";
        std::string readContent = FileSystem::ReadStringFromFile(testFile);
        if (readContent == testContent)
        {
            std::cout << "文件读取成功，内容匹配\n";
            std::cout << "读取内容: " << readContent << std::endl;
        }

        // 4. 测试文件信息获取
        std::cout << "\n4. 测试文件信息获取\n";
        ST_FileInfo fileInfo = FileSystem::GetFileInfo(testFile);
        std::cout << "文件名: " << fileInfo.m_name << std::endl;
        std::cout << "文件大小: " << fileInfo.m_size << " 字节" << std::endl;
        std::cout << "是否是目录: " << (fileInfo.m_isDirectory ? "是" : "否") << std::endl;
        std::cout << "是否只读: " << (fileInfo.m_isReadOnly ? "是" : "否") << std::endl;

        // 5. 测试文件复制
        std::cout << "\n5. 测试文件复制\n";
        if (FileSystem::Copy(testFile, testFile2, true))
        {
            std::cout << "文件复制成功: " << testFile << " -> " << testFile2 << std::endl;
        }

        // 6. 测试文件属性设置
        std::cout << "\n6. 测试文件属性设置\n";
        if (FileSystem::SetFileAttributes(testFile, true))
        {
            std::cout << "设置文件只读属性成功\n";
            fileInfo = FileSystem::GetFileInfo(testFile);
            std::cout << "文件只读状态: " << (fileInfo.m_isReadOnly ? "是" : "否") << std::endl;
        }

        // 7. 测试目录内容获取
        std::cout << "\n7. 测试目录内容获取\n";
        auto files = FileSystem::GetFiles(testDir, false);
        std::cout << "目录中的文件:\n";
        for (const auto& file : files)
        {
            std::cout << "  " << file << std::endl;
        }

        // 8. 测试文件删除
        std::cout << "\n8. 测试文件删除\n";
        if (FileSystem::Delete(testFile2))
        {
            std::cout << "文件删除成功: " << testFile2 << std::endl;
        }

        // 9. 测试文件名操作
        std::cout << "\n9. 测试文件名操作\n";
        std::cout << "文件扩展名: " << FileSystem::GetExtension(testFile) << std::endl;
        std::cout << "不带扩展名的文件名: " << FileSystem::GetFileNameWithoutExtension(testFile) << std::endl;

        // 10. 测试JSON操作
        std::cout << "\n10. 测试JSON操作\n";
        std::string jsonStr = R"({"name": "测试", "value": 123})";
        std::string jsonFile = testDir + "/test.json";
        
        if (FileSystem::WriteJsonToFile(jsonFile, jsonStr, true) == EM_JsonOperationResult::Success)
        {
            std::cout << "JSON文件写入成功\n";
            std::string readJson;
            if (FileSystem::ReadJsonFromFile(jsonFile, readJson) == EM_JsonOperationResult::Success)
            {
                std::cout << "JSON文件读取成功: " << readJson << std::endl;
            }
        }

        std::cout << "\n=== 文件系统测试完成 ===\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "测试过程中发生错误: " << e.what() << std::endl;
    }
} 