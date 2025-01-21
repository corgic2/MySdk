#include <iostream>
#include <string>
#include "FileSystem.h"

void FileSystemTest()
{
    std::cout << "Testing FileSystem..." << std::endl;
    // 测试路径
    const std::string testFilePath = "./test";
    const std::string testFileName = "testfile.txt";
    const std::string testFolderName = "testfolder";
    const std::string testString = "Hello, FileSystem!";
    // 测试文件夹是否存在
    std::cout << "Testing FolderIsExists..." << std::endl;

    if (!FileSystem::FolderIsExists(testFilePath))
    {
        std::cout << "Folder does not exist. Attempting to create it..." << std::endl;

        if (FileSystem::TryCreateFolder(testFilePath,testFileName))
        {
            std::cout << "Folder created successfully." << std::endl;
        }
        else
        {
            std::cout << "Failed to create folder." << std::endl;
            return;
        }
    }
    else
    {
        std::cout << "Folder already exists." << std::endl;
    }

    // 测试文件是否存在
    std::cout << "Testing FileIsExists..." << std::endl;

    if (!FileSystem::FileIsExists(testFilePath, testFileName))
    {
        std::cout << "File does not exist. Attempting to create it..." << std::endl;

        if (FileSystem::TryCreateFile(testFilePath, testFileName))
        {
            std::cout << "File created successfully." << std::endl;
        }
        else
        {
            std::cout << "Failed to create file." << std::endl;
            return;
        }
    }
    else
    {
        std::cout << "File already exists." << std::endl;
    }

    // 测试写入文件
    std::cout << "Testing WriteStringToFile..." << std::endl;

    if (FileSystem::WriteStringToFile(testFilePath, testFileName, testString))
    {
        std::cout << "String written to file successfully." << std::endl;
    }
    else
    {
        std::cout << "Failed to write string to file." << std::endl;
        return;
    }

    // 测试读取文件
    std::cout << "Testing ReadStringFromFile..." << std::endl;
    std::string readData;
    FileSystem::ReadStringFromFile(testFilePath, testFileName, readData);
    std::cout << "Read from file: " << readData << std::endl;
    // 测试删除文件
    std::cout << "Testing TryDeleteFile..." << std::endl;

    if (FileSystem::TryDeleteFile(testFilePath, testFileName))
    {
        std::cout << "File deleted successfully." << std::endl;
    }
    else
    {
        std::cout << "Failed to delete file." << std::endl;
    }

    // 测试删除文件夹
    std::cout << "Testing TryDeleteFolder..." << std::endl;

    if (FileSystem::TryDeleteFolder(testFilePath))
    {
        std::cout << "Folder deleted successfully." << std::endl;
    }
    else
    {
        std::cout << "Failed to delete folder." << std::endl;
    }

    std::cout << "All tests completed." << std::endl;
}

int main()
{
    FileSystemTest();
    return 0;
}