// ************************************************************
// <remarks>
// Author      : liaofeng
// CreateTime  : 2025-01-07
// Description : 文件系统工具类
// </remarks>
// ************************************************************

#include <string>

class FileSystem
{
    FileSystem();
    ~FileSystem();
    //读写文件
    static bool WriteStringToFile(std::string str, std::string filePath);
    static void ReadStringFromFile(std::string filePath, std::string& outData);
    //文件存在
    static bool FileIsExists(std::string filePath);
    static bool FolderIsExists(std::string path);
    //创建文件
    static bool TryCreateFile(std::string path, std::string name);
    static bool TryCreateFolder(std::string path, std::string folderName);
    //删除文件
    static bool DeleteFile(std::string path);
    static bool DeleteFolder(std::string path);
};