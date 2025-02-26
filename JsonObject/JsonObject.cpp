#include "JsonObject.h"

#include <filesystem>
#include "../SDKCommonDefine/SDKCommonDefine.h"

my_sdk::JsonValue::JsonValue()
{
}

my_sdk::JsonValue::~JsonValue()
{
}

my_sdk::JsonValue::JsonValue(const JsonValue& obj)
{
    *this = obj;
}

my_sdk::JsonValue& my_sdk::JsonValue::operator=(const JsonValue& obj)
{
    if (this != &obj)
    {
        Clear();
        m_strValue = obj.m_strValue;
        m_booleanValue = obj.m_booleanValue;
        m_numberValue = obj.m_numberValue;
        m_pointerValue = obj.m_pointerValue;

        for (auto i = m_mapValue.begin(); i != m_mapValue.end(); ++i)
        {
            m_mapValue[i->first] = i->second;
        }
    }

    return *this;
}

void my_sdk::JsonValue::Clear()
{
    m_strValue = "";
    m_booleanValue = false;
    m_numberValue = 0.0;
    m_mapValue.clear();
}

my_sdk::JsonObjectPrivate::JsonObjectPrivate()
{
    SetValueType(Object);
}

my_sdk::JsonObjectPrivate::~JsonObjectPrivate()
{
    SAFE_DELETE_POINTER_VALUE(m_jsonObject)
}

my_sdk::JsonObjectPrivate::JsonObjectPrivate(const JsonObjectPrivate& jsonObj)
{
    *this = jsonObj;
}

my_sdk::JsonObjectPrivate& my_sdk::JsonObjectPrivate::operator=(const JsonObjectPrivate& jsonObj)
{
    if (this != &jsonObj)
    {
        SAFE_DELETE_POINTER_VALUE(m_jsonObject)
        m_jsonObject = new JsonValue(*jsonObj.m_jsonObject);
        m_valueType = jsonObj.m_valueType;
    }

    return *this;
}

my_sdk::JsonObjectPrivate my_sdk::JsonObjectPrivate::ParseObject(const std::string& content, size_t& index)
{
    JsonObjectPrivate result;
    result.SetValueType(EM_JsonValue::Object);

    while (index < content.size())
    {
        char ch = content[index];

        if (ch == '}')
        {
            ++index; // 跳过闭合的 '}'
            break;
        }
        else if (ch == '"')
        {
            ++index; // 跳过开始的 '"'
            size_t keyStart = index;

            while (index < content.size() && content[index] != '"')
            {
                ++index;
            }

            std::string key = content.substr(keyStart, index - keyStart);
            ++index; // 跳过结束的 '"'

            // 跳过冒号
            while (index < content.size() && (content[index] == ' ' || content[index] == ':'))
            {
                ++index;
            }

            // 解析值
            JsonObjectPrivate value = ParseValue(content, index);
            result.AddJsonObj(key, value);

            // 跳过逗号
            while (index < content.size() && (content[index] == ' ' || content[index] == ','))
            {
                ++index;
            }
        }
        else
        {
            ++index; // 跳过其他字符
        }
    }

    return result;
}

my_sdk::JsonObjectPrivate my_sdk::JsonObjectPrivate::ParseArray(const std::string& content, size_t& index)
{
    JsonObjectPrivate result;
    result.SetValueType(EM_JsonValue::Array);

    while (index < content.size())
    {
        char ch = content[index];

        if (ch == ']')
        {
            ++index; // 跳过闭合的 ']'
            break;
        }
        else if (ch == ' ' || ch == ',')
        {
            ++index; // 跳过空格和逗号
        }
        else
        {
            // 解析数组元素
            JsonObjectPrivate value = ParseValue(content, index);
            result.AddJsonObj(std::to_string(result.GetValue().m_mapValue.size()), value);
        }
    }

    return result;
}

my_sdk::JsonObjectPrivate my_sdk::JsonObjectPrivate::ParseValue(const std::string& content, size_t& index)
{
    JsonObjectPrivate result;

    while (index < content.size())
    {
        char ch = content[index];

        if (ch == '{')
        {
            ++index; // 跳过开始的 '{'
            result = ParseObject(content, index);
            break;
        }
        else if (ch == '[')
        {
            ++index; // 跳过开始的 '['
            result = ParseArray(content, index);
            break;
        }
        else if (ch == '"')
        {
            ++index; // 跳过开始的 '"'
            size_t start = index;

            while (index < content.size() && content[index] != '"')
            {
                ++index;
            }

            result.GetValue().m_strValue = content.substr(start, index - start);
            result.SetValueType(EM_JsonValue::String);
            ++index; // 跳过结束的 '"'
            break;
        }
        else if (ch == 't' || ch == 'f')
        {
            // 解析布尔值
            if (content.substr(index, 4) == "true")
            {
                result.GetValue().m_booleanValue = true;
                index += 4;
            }
            else if (content.substr(index, 5) == "false")
            {
                result.GetValue().m_booleanValue = false;
                index += 5;
            }

            result.SetValueType(EM_JsonValue::Boolean);
            break;
        }
        else if (ch == 'n')
        {
            // 解析null
            if (content.substr(index, 4) == "null")
            {
                result.GetValue().m_pointerValue = nullptr;
                index += 4;
            }

            result.SetValueType(EM_JsonValue::Null);
            break;
        }
        else if (ch == '-' || (ch >= '0' && ch <= '9'))
        {
            // 解析数字
            size_t start = index;

            while (index < content.size() && (content[index] == '-' || content[index] == '.' || (
                                                  content[index] >= '0' && content[index] <= '9')))
            {
                ++index;
            }

            result.GetValue().m_numberValue = std::stod(content.substr(start, index - start));
            result.SetValueType(EM_JsonValue::Number);
            break;
        }
        else
        {
            ++index; // 跳过其他字符
        }
    }

    return result;
}

void my_sdk::JsonObjectPrivate::SetValueType(const EM_JsonValue& enumValue)
{
    m_valueType = enumValue;
}

my_sdk::EM_JsonValue my_sdk::JsonObjectPrivate::GetValueType() const
{
    return m_valueType;
}

my_sdk::JsonObjectPrivate my_sdk::JsonObjectPrivate::GetJsonObj(const std::string& key) const
{
    if (m_valueType != Object || !m_jsonObject)
    {
        return JsonObjectPrivate();
    }

    auto it = m_jsonObject->m_mapValue.find(key);

    if (it != m_jsonObject->m_mapValue.end())
    {
        return it->second;
    }
    return JsonObjectPrivate();
}

void my_sdk::JsonObjectPrivate::SetJsonObjValue(const std::string& key, const JsonObjectPrivate& jsonObject)
{
    if (m_valueType != Object || !m_jsonObject)
    {
        return;
    }

    //存在就更改，不存在就不做处理
    if (m_jsonObject->m_mapValue.count(key))
    {
        m_jsonObject->m_mapValue[key] = jsonObject;
    }
}

void my_sdk::JsonObjectPrivate::AddJsonObj(const std::string& key, const JsonObjectPrivate& jsonObject)
{
    if (m_valueType != Object || !m_jsonObject)
    {
        return;
    }

    //不存在就添加，存在就不做处理
    if (!m_jsonObject->m_mapValue.count(key))
    {
        m_jsonObject->m_mapValue[key] = jsonObject;
    }
}

my_sdk::JsonValue& my_sdk::JsonObjectPrivate::GetValue() const
{
    if (m_jsonObject)
    {
        return *m_jsonObject;
    }
    static JsonValue emptyObject;
    return emptyObject;
}

void my_sdk::JsonObjectPrivate::SetValue(const JsonValue& value)
{
    SAFE_DELETE_POINTER_VALUE(m_jsonObject)
    m_jsonObject = new JsonValue(value);
}

my_sdk::JsonObject::JsonObject()
{
}

my_sdk::JsonObject::JsonObject(const std::string& filePath, const std::string& fileName)
{
    FileSystem::ReadStringFromFile(filePath, fileName, m_content);
    size_t index = 0;
    m_obj = m_obj.ParseObject(m_content, index);
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
