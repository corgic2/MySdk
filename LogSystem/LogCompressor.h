#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <ctime>

/// <summary>
/// 日志压缩等级枚举
/// </summary>
enum class EM_CompressLevel
{
    None = 0,    ///< 不压缩
    Fast = 1,    ///< 快速压缩
    Normal = 6,  ///< 普通压缩
    Best = 9     ///< 最佳压缩
};

/// <summary>
/// 简单的游程编码压缩器
/// </summary>
class LogCompressor
{
public:
    /// <summary>
    /// 压缩数据
    /// </summary>
    /// <param name="data">原始数据</param>
    /// <param name="level">压缩等级</param>
    /// <returns>压缩后的数据</returns>
    static std::vector<char> Compress(const std::string& data, EM_CompressLevel level = EM_CompressLevel::Fast)
    {
        if (data.empty() || level == EM_CompressLevel::None)
        {
            return std::vector<char>(data.begin(), data.end());
        }

        std::vector<char> compressed;
        compressed.reserve(data.size());

        for (size_t i = 0; i < data.size(); ++i)
        {
            char currentChar = data[i];
            size_t count = 1;

            while (i + 1 < data.size() && data[i + 1] == currentChar && count < 255)
            {
                ++count;
                ++i;
            }

            if (count > 3 || currentChar == '\0')
            {
                compressed.push_back('\0');  // 转义字符
                compressed.push_back(static_cast<char>(count));
                compressed.push_back(currentChar);
            }
            else
            {
                for (size_t j = 0; j < count; ++j)
                {
                    compressed.push_back(currentChar);
                }
            }
        }

        return compressed;
    }

    /// <summary>
    /// 解压数据
    /// </summary>
    /// <param name="compressedData">压缩数据</param>
    /// <returns>解压后的数据</returns>
    static std::string Decompress(const std::vector<char>& compressedData)
    {
        if (compressedData.empty())
        {
            return "";
        }

        std::string decompressed;
        decompressed.reserve(compressedData.size() * 2);

        for (size_t i = 0; i < compressedData.size(); ++i)
        {
            if (compressedData[i] == '\0' && i + 2 < compressedData.size())
            {
                size_t count = static_cast<unsigned char>(compressedData[i + 1]);
                char value = compressedData[i + 2];
                decompressed.append(count, value);
                i += 2;
            }
            else
            {
                decompressed.push_back(compressedData[i]);
            }
        }

        return decompressed;
    }
};

/// <summary>
/// 压缩日志块结构体
/// </summary>
struct ST_CompressedLogBlock
{
    std::vector<char> m_data;    ///< 压缩数据
    size_t m_originalSize;       ///< 原始大小
    time_t m_timestamp;          ///< 时间戳

    /// <summary>
    /// 构造函数
    /// </summary>
    ST_CompressedLogBlock()
        : m_originalSize(0)
        , m_timestamp(0)
    {
    }

    /// <summary>
    /// 写入压缩块到文件
    /// </summary>
    /// <param name="file">文件流</param>
    void WriteToFile(std::ofstream& file) const
    {
        // 写入块头
        file.write(reinterpret_cast<const char*>(&m_originalSize), sizeof(m_originalSize));
        file.write(reinterpret_cast<const char*>(&m_timestamp), sizeof(m_timestamp));
        
        // 写入压缩数据大小
        size_t compressedSize = m_data.size();
        file.write(reinterpret_cast<const char*>(&compressedSize), sizeof(compressedSize));
        
        // 写入压缩数据
        file.write(m_data.data(), compressedSize);
    }

    /// <summary>
    /// 从文件读取压缩块
    /// </summary>
    /// <param name="file">文件流</param>
    /// <returns>是否成功</returns>
    bool ReadFromFile(std::ifstream& file)
    {
        // 读取块头
        if (!file.read(reinterpret_cast<char*>(&m_originalSize), sizeof(m_originalSize)))
        {
            return false;
        }
        if (!file.read(reinterpret_cast<char*>(&m_timestamp), sizeof(m_timestamp)))
        {
            return false;
        }

        // 读取压缩数据大小
        size_t compressedSize;
        if (!file.read(reinterpret_cast<char*>(&compressedSize), sizeof(compressedSize)))
        {
            return false;
        }

        // 读取压缩数据
        m_data.resize(compressedSize);
        if (!file.read(m_data.data(), compressedSize))
        {
            return false;
        }

        return true;
    }
}; 