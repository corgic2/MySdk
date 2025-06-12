// ************************************************************
// <remarks>
// Author      : liaofeng
// CreateTime  : 2025-01-07
// Description : 文件系统工具类，Windows系统下
// </remarks>
// ************************************************************
#pragma once
#include <string>
#include <vector>
#include <ctime>
#include "../SDKCommonDefine/SDK_Export.h"

namespace my_sdk
{
    /// <summary>
    /// JSON操作结果枚举
    /// </summary>
    enum class EM_JsonOperationResult
    {
        Success,            /// 操作成功
        FileNotFound,       /// 文件不存在
        InvalidJson,        /// 无效的JSON格式
        WriteError,         /// 写入错误
        ReadError          /// 读取错误
    };

    /// <summary>
    /// 音频文件类型枚举
    /// </summary>
    enum class EM_AudioFileType
    {
        Unknown,    /// 未知类型
        MP3,        /// MP3格式
        WAV,        /// WAV格式
        FLAC,       /// FLAC格式
        M4A         /// M4A格式
    };

    /// <summary>
    /// 文件信息结构体
    /// </summary>
    struct ST_FileInfo
    {
        std::string m_name;          ///< 文件名
        std::string m_path;          ///< 完整路径
        size_t m_size;               ///< 文件大小（字节）
        time_t m_createTime;         ///< 创建时间
        time_t m_modifyTime;         ///< 修改时间
        time_t m_accessTime;         ///< 访问时间
        bool m_isDirectory;          ///< 是否是目录
        bool m_isReadOnly;           ///< 是否只读

        /// <summary>
        /// 构造函数
        /// </summary>
        ST_FileInfo()
            : m_size(0)
            , m_createTime(0)
            , m_modifyTime(0)
            , m_accessTime(0)
            , m_isDirectory(false)
            , m_isReadOnly(false)
        {
        }
    };

    /// <summary>
    /// 音频文件信息结构体
    /// </summary>
    struct ST_AudioFileInfo : public ST_FileInfo
    {
        EM_AudioFileType m_fileType;     /// 音频文件类型
        std::string m_displayName;        /// 显示名称
        std::string m_iconPath;          /// 图标路径

        /// <summary>
        /// 构造函数
        /// </summary>
        ST_AudioFileInfo() : ST_FileInfo(), m_fileType(EM_AudioFileType::Unknown)
        {
        }
    };

    /// <summary>
    /// 文件系统工具类
    /// </summary>
    class SDK_API FileSystem
    {
    public:
        FileSystem();
        ~FileSystem();

        /// <summary>
        /// 写入字符串到文件
        /// </summary>
        /// <param name="filePath">文件路径</param>
        /// <param name="str">要写入的字符串</param>
        /// <param name="writeBom">是否写入BOM</param>
        /// <returns>是否成功</returns>
        static bool WriteStringToFile(const std::string& filePath, const std::string& str, bool writeBom = true);

        /// <summary>
        /// 从文件读取字符串
        /// </summary>
        /// <param name="filePath">文件路径</param>
        /// <param name="removeBOM">是否移除BOM</param>
        /// <returns>读取的字符串</returns>
        static std::string ReadStringFromFile(const std::string& filePath, bool removeBOM = true);

        /// <summary>
        /// 转换编码为UTF-8
        /// </summary>
        /// <param name="input">输入字符串</param>
        /// <param name="to">目标编码</param>
        /// <returns>转换后的字符串</returns>
        static std::string ConvertEncodingToUtf_8(const std::string& input, const std::string& to = "UTF-8");

        /// <summary>
        /// 列出目录内容
        /// </summary>
        /// <param name="dir">目录路径</param>
        static void ListDirectory(const std::string& dir);

        /// <summary>
        /// 获取文件信息
        /// </summary>
        /// <param name="path">文件路径</param>
        /// <returns>文件信息结构体</returns>
        static ST_FileInfo GetFileInfo(const std::string& path);

        /// <summary>
        /// 复制文件或目录
        /// </summary>
        /// <param name="source">源路径</param>
        /// <param name="destination">目标路径</param>
        /// <param name="overwrite">是否覆盖已存在的文件</param>
        /// <returns>是否成功</returns>
        static bool Copy(const std::string& source, const std::string& destination, bool overwrite = false);

        /// <summary>
        /// 移动文件或目录
        /// </summary>
        /// <param name="source">源路径</param>
        /// <param name="destination">目标路径</param>
        /// <returns>是否成功</returns>
        static bool Move(const std::string& source, const std::string& destination);

        /// <summary>
        /// 删除文件或目录
        /// </summary>
        /// <param name="path">要删除的路径</param>
        /// <returns>是否成功</returns>
        static bool Delete(const std::string& path);

        /// <summary>
        /// 创建目录
        /// </summary>
        /// <param name="path">目录路径</param>
        /// <returns>是否成功</returns>
        static bool CreateDirectory(const std::string& path);

        /// <summary>
        /// 判断文件或目录是否存在
        /// </summary>
        /// <param name="path">路径</param>
        /// <returns>是否存在</returns>
        static bool Exists(const std::string& path);

        /// <summary>
        /// 获取文件大小
        /// </summary>
        /// <param name="path">文件路径</param>
        /// <returns>文件大小（字节）</returns>
        static size_t GetFileSize(const std::string& path);

        /// <summary>
        /// 获取目录下所有文件
        /// </summary>
        /// <param name="dir">目录路径</param>
        /// <param name="recursive">是否递归搜索子目录</param>
        /// <returns>文件路径列表</returns>
        static std::vector<std::string> GetFiles(const std::string& dir, bool recursive = false);

        /// <summary>
        /// 获取目录下所有子目录
        /// </summary>
        /// <param name="dir">目录路径</param>
        /// <param name="recursive">是否递归搜索子目录</param>
        /// <returns>目录路径列表</returns>
        static std::vector<std::string> GetDirectories(const std::string& dir, bool recursive = false);

        /// <summary>
        /// 设置文件属性
        /// </summary>
        /// <param name="path">文件路径</param>
        /// <param name="readOnly">是否只读</param>
        /// <returns>是否成功</returns>
        static bool SetFileAttributes(const std::string& path, bool readOnly);

        /// <summary>
        /// 获取文件扩展名
        /// </summary>
        /// <param name="path">文件路径</param>
        /// <returns>扩展名</returns>
        static std::string GetExtension(const std::string& path);

        /// <summary>
        /// 获取不带扩展名的文件名
        /// </summary>
        /// <param name="path">文件路径</param>
        /// <returns>文件名</returns>
        static std::string GetFileNameWithoutExtension(const std::string& path);

        /// <summary>
        /// 将JSON字符串写入文件
        /// </summary>
        /// <param name="filePath">文件路径</param>
        /// <param name="jsonStr">JSON字符串</param>
        /// <param name="pretty">是否格式化JSON</param>
        /// <returns>操作结果</returns>
        static EM_JsonOperationResult WriteJsonToFile(const std::string& filePath, const std::string& jsonStr, bool pretty = true);

        /// <summary>
        /// 从文件读取JSON字符串
        /// </summary>
        /// <param name="filePath">文件路径</param>
        /// <param name="jsonStr">输出的JSON字符串</param>
        /// <returns>操作结果</returns>
        static EM_JsonOperationResult ReadJsonFromFile(const std::string& filePath, std::string& jsonStr);

        /// <summary>
        /// 验证JSON字符串格式是否有效
        /// </summary>
        /// <param name="jsonStr">要验证的JSON字符串</param>
        /// <returns>true表示有效，false表示无效</returns>
        static bool ValidateJsonString(const std::string& jsonStr);

        /// <summary>
        /// 格式化JSON字符串
        /// </summary>
        /// <param name="jsonStr">要格式化的JSON字符串</param>
        /// <param name="formattedStr">格式化后的字符串</param>
        /// <returns>true表示格式化成功，false表示失败</returns>
        static bool FormatJsonString(const std::string& jsonStr, std::string& formattedStr);

        /// <summary>
        /// 判断文件是否为音频文件
        /// </summary>
        /// <param name="filePath">文件路径</param>
        /// <returns>true表示是音频文件，false表示不是</returns>
        static bool IsAudioFile(const std::string& filePath);

        /// <summary>
        /// 获取音频文件类型
        /// </summary>
        /// <param name="filePath">文件路径</param>
        /// <returns>音频文件类型</returns>
        static EM_AudioFileType GetAudioFileType(const std::string& filePath);

        /// <summary>
        /// 获取音频文件信息
        /// </summary>
        /// <param name="filePath">文件路径</param>
        /// <returns>音频文件信息</returns>
        static ST_AudioFileInfo GetAudioFileInfo(const std::string& filePath);

        /// <summary>
        /// 获取目录下所有音频文件
        /// </summary>
        /// <param name="dir">目录路径</param>
        /// <param name="recursive">是否递归搜索子目录</param>
        /// <returns>音频文件路径列表</returns>
        static std::vector<std::string> GetAudioFiles(const std::string& dir, bool recursive = false);

        /// <summary>
        /// 获取支持的音频文件扩展名列表
        /// </summary>
        /// <returns>扩展名列表（包含点号，如 .mp3）</returns>
        static std::vector<std::string> GetSupportedAudioExtensions();

        /// <summary>
        /// 获取音频文件过滤器（用于文件对话框）
        /// </summary>
        /// <returns>过滤器字符串</returns>
        static std::string GetAudioFileFilter();

        /// <summary>
        /// 将Qt路径转换为标准路径
        /// </summary>
        /// <param name="qtPath">Qt路径</param>
        /// <returns>标准路径</returns>
        static std::string QtPathToStdPath(const std::string& qtPath);

        /// <summary>
        /// 将标准路径转换为Qt路径
        /// </summary>
        /// <param name="stdPath">标准路径</param>
        /// <returns>Qt路径</returns>
        static std::string StdPathToQtPath(const std::string& stdPath);

        /// <summary>
        /// 转义JSON字符串
        /// </summary>
        /// <param name="str">要转义的字符串</param>
        /// <returns>转义后的字符串</returns>
        static std::string EscapeJsonString(const std::string& str);

        /// <summary>
        /// 反转义JSON字符串
        /// </summary>
        /// <param name="str">要反转义的字符串</param>
        /// <returns>反转义后的字符串</returns>
        static std::string UnescapeJsonString(const std::string& str);

    private:
        /// <summary>
        /// 递归获取目录内容
        /// </summary>
        /// <param name="dir">目录路径</param>
        /// <param name="files">文件列表</param>
        /// <param name="directories">目录列表</param>
        static void GetDirectoryContents(const std::string& dir, 
                                       std::vector<std::string>& files,
                                       std::vector<std::string>& directories);
    };
}
