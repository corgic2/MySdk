#include "FileSystem.h"

FileSystem::FileSystem()
{
}

FileSystem::~FileSystem()
{
}

bool FileSystem::WriteStringToFile(std::string str, std::string filePath)
{
    return true;
}

void FileSystem::ReadStringFromFile(std::string filePath, std::string& outData)
{
}

bool FileSystem::FileIsExists(std::string filePath)
{
    return true;
}

bool FileSystem::FolderIsExists(std::string path)
{
    return true;
}

bool FileSystem::TryCreateFile(std::string path, std::string name)
{
    return true;
}

bool FileSystem::TryCreateFolder(std::string path, std::string folderName)
{
    return true;
}

bool FileSystem::DeleteFile(std::string path)
{
    return true;
}

bool FileSystem::DeleteFolder(std::string path)
{
    return true;
}
