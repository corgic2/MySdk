// ************************************************************
// <remarks>
// Author      : liaofeng
// CreateTime  : 2025-01-07
// Description : 文件系统工具类，Windows系统下
// </remarks>
// ************************************************************
#pragma once
#include <string>
#include <boost/filesystem/path.hpp>
#include "../SDKCommonDefine/SDK_Export.h"

namespace my_sdk
{
  class FileSystemUtils
  {
  };


  class SDK_API FileSystem
  {
  public:
    FileSystem();
    ~FileSystem();

    // 读写文件
    static bool WriteStringToFile(const boost::filesystem::path& filePath, const std::string& str, bool writeBom = false);

    // 写入内容str至路径filePath下文件名fileName的文件
    static std::string ReadStringFromFile(const boost::filesystem::path& filePath, bool removeBOM = true);

    static std::string ConvertEncodingToUtf_8(const std::string& input, const std::string& to = "UTF-8");

    //列出次目录下所有的文件
    static void ListDirectory(const boost::filesystem::path& dir);

private:

    // 获取目录下所有文件的路径
    std::vector<boost::filesystem::path> Getfiles(const boost::filesystem::path& dir);
  };
}
