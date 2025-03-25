#include "JsonObject.h"
#include "JsonObjectPrivate.h"
#include <filesystem>
#include "../SDKCommonDefine/SDKCommonDefine.h"

my_sdk::JsonObject::JsonObject()
{
}

my_sdk::JsonObject::JsonObject(const std::string& filePath, const std::string& fileName)
{
    FileSystem::ReadStringFromFile(filePath, fileName, m_content);
    size_t index = 0;

    while (m_content[index] != '{')
    {
        ++index;
    }

    m_obj = m_obj.ParseObject(m_content, ++index);
}

my_sdk::JsonObject::~JsonObject()
{
}

my_sdk::JsonObject& my_sdk::JsonObject::operator=(const JsonObject& obj)
{
    if (this != &obj)
    {
        m_obj = obj.m_obj;
        m_content = obj.m_content;
    }

    return *this;
}

void my_sdk::JsonObject::SetJsonObject(const JsonObject& obj)
{
    *this = obj;
}

my_sdk::JsonObjectPrivate& my_sdk::JsonObject::GetJsonObject()
{
    return m_obj;
}