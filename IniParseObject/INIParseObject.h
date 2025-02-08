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
        // �洢�����������
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sections;
        // ����������ȥ���ַ������˵Ŀհ��ַ�
        std::string Trim(const std::string& str);
        // �����������ж��Ƿ���ע����
        bool IsComment(const std::string& line);
    public:
        INIParseObject() = default;
        ~INIParseObject() = default;
        // ���ز�����INI�ļ�
        bool Load(const std::string& filePath);
        // ��ȡָ�����ε����м�ֵ��
        std::unordered_map<std::string, std::string> GetSection(const std::string& sectionName) const;
        // ��ȡָ�����ε�ָ������ֵ
        std::string GetValue(const std::string& sectionName, const std::string& keyName) const;
        // ��ӡ���н���������ݣ����ڵ��ԣ�
        void PrintAll() const;
    };
}