#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
namespace my_sdk
{
    class INIParseObject
    {
    public:
        // 存储解析后的数据
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sections;
        // 辅助函数：去除字符串两端的空白字符
        std::string Trim(const std::string& str);
        // 辅助函数：判断是否是注释行
        bool IsComment(const std::string& line);
    public:
        INIParseObject() = default;
        ~INIParseObject() = default;
        // 加载并解析INI文件
        bool Load(const std::string& filePath);
        // 获取指定区段的所有键值对
        std::unordered_map<std::string, std::string> GetSection(const std::string& sectionName) const;
        // 获取指定区段的指定键的值
        std::string GetValue(const std::string& sectionName, const std::string& keyName) const;
        // 打印所有解析后的数据（用于调试）
        void PrintAll() const;
    };
}