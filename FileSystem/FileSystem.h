// ************************************************************
// <remarks>
// Author      : liaofeng
// CreateTime  : 2025-01-07
// Description : 文件系统工具类，Windows系统下
// </remarks>
// ************************************************************

#include <string>

class FileSystem
{
public:
    FileSystem();
    ~FileSystem();
    //读写文件
    static bool WriteStringToFile(const std::string& filePath, const std::string& fileName, const std::string& str);   //写入内容str至路径filePath下文件名fileName的文件
    static void ReadStringFromFile(const std::string& filePath, const std::string& fileName, std::string& outData);   //读取路径filePath下文件名为filename的文件内容
    //文件存在
    static bool FileIsExists(const std::string& filePath, const std::string& fileName);  // 判断路径为filePath下文件名为fineName的文件是否存在
    static bool FolderIsExists(const std::string& path); //判断path路径下的文件夹是否存在
    //创建文件
    static bool TryCreateFile(const std::string& path, const std::string& name); //尝试创建路径path下文件名为name的文件
    static bool TryCreateFolder(const std::string& path, const std::string& folderName); //尝试创建路径path下名为folderName的文件夹
    //删除文件
    static bool TryDeleteFile(const std::string& filePath, const std::string& fileName); //尝试删除路径filePath下文件名为fileName的文件
    static bool TryDeleteFolder(const std::string& path);//尝试删除路径Path下的文件夹
};