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
    bool WriteStringToFile(std::string str, std::string filePath);
    std::string ReadStringFromFile(std::string filePath);
    //文件存在
    bool FileIsExists(std::string filePath);
    bool FolderIsExists(std::string path);
    //创建文件
    bool TryCreateFile(std::string path, std::string name);
    bool TryCreateFolder(std::string path, std::string folderName);
    //删除文件
    bool DeleteFile(std::string path);
    bool DeleteFolder(std::string path);
};