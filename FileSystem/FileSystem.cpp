#include <fstream>
#include <iostream>
#include <cstdlib>
#include <windows.h>
#include "FileSystem.h"


FileSystem::FileSystem()
{
}

FileSystem::~FileSystem()
{
}

bool FileSystem::WriteStringToFile(const std::string& filePath, const std::string& fileName, const std::string& str)
{
    
    if(false == FolderIsExists(filePath))
    {
        return false;
    }

    if(false == FileIsExists(filePath,fileName))
    {
        return false;   
    }

    std::ofstream file(filePath + "/" + fileName);
    //文件流没有被打开
    
    if(!file.is_open())
    {
        return false;
    }

    //写入
    file<<str;
    file.close();
    return true;
}

void FileSystem::ReadStringFromFile(const std::string& filePath, const std::string& fileName, std::string& outData)
{
}

bool FileSystem::FileIsExists(const std::string& filePath, const std::string& fileName)
{

    if(false == FolderIsExists(filePath))
    {
        return false;
    }

    //在打开文件时若文件不存在则默认会创建文件
    std::ifstream file(filePath);
    bool bOpen = file.is_open();
    file.close();
    return bOpen;
}

bool FileSystem::FolderIsExists(const std::string& path)
{
    DWORD attributes = GetFileAttributesA(path.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY));
}

bool FileSystem::TryCreateFile(const std::string& path, const std::string& name)
{
    
    if(false == FolderIsExists(path))
    {
        return false;
    }

    return FileIsExists(path,name);
}

bool FileSystem::TryCreateFolder(const std::string& path, const std::string& folderName)
{

    if(false == FolderIsExists(path))
    {
        return false;
    }

    int ret = system(("cd " + path + "\n" + "mkdir " + folderName).c_str());
    return ret == 0;
}

bool FileSystem::TryDeleteFile(const std::string& filePath,const std::string& fileName)
{

    if(false == FolderIsExists(filePath))
    {
        return false;
    }

    return std::remove((filePath + "/" +  fileName).c_str());
}

bool FileSystem::TryDeleteFolder(const std::string& filePath)
{

    if(false == FolderIsExists(filePath))
    {
        return false;
    }

    int ret = system(("rmdir /s /q " + filePath).c_str());
    return ret == 0;
}
