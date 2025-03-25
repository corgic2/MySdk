// ************************************************************
// <remarks>
// Author      : liaofeng
// CreateTime  : 2025-01-07
// Description : 文件系统工具类，Windows系统下
// </remarks>
// ************************************************************
#pragma once
#include <string>
#pragma execution_character_set("utf-8")
namespace my_sdk
{
  class FileSystemUtils
  {
  public:
    static std::string CombinePath(const std::string& path1, const std::string& path2);
  };


  class FileSystem
  {
  public:
    FileSystem();
    ~FileSystem();
    // 读写文件
    static bool WriteStringToFile(const std::string& filePath, const std::string& fileName, const std::string& str);
    // 写入内容str至路径filePath下文件名fileName的文件
    static void ReadStringFromFile(const std::string& filePath, const std::string& fileName, std::string& outData);
    // 读取路径filePath下文件名为filename的文件内容
  };
}
