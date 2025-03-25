#include "JsonObjectPrivate.h"

#include <iostream>
#include <ostream>

my_sdk::JsonObjectPrivate::JsonObjectPrivate()
{
    SetValueType(JsonValue::EM_JsonValue::Object);
}

my_sdk::JsonObjectPrivate::~JsonObjectPrivate()
{
}

my_sdk::JsonObjectPrivate::JsonObjectPrivate(const JsonObjectPrivate& jsonObj)
{
    *this = jsonObj;
}

my_sdk::JsonObjectPrivate& my_sdk::JsonObjectPrivate::operator=(const JsonObjectPrivate& jsonObj)
{
    if (this != &jsonObj)
    {
        m_object = jsonObj.m_object;
        m_valueType = jsonObj.m_valueType;
        m_JsonValueArray = jsonObj.m_JsonValueArray;
        m_Jsonvalue = jsonObj.m_Jsonvalue;
    }

    return *this;
}

my_sdk::JsonObjectPrivate my_sdk::JsonObjectPrivate::ParseObject(const std::string& content, size_t& index)
{
    std::string key;
    while (index < content.size())
    {
        // "123" : "1234"
        // "123" : false
        // "123" : [ "1234" : "11" ]
        // "123" : { "123" : "123" }

        if (content[index] != '"')
        {
            ++index;
        }
        else // 找到key的第一个 "
        {
            //找到key的下一个 "
            while (content[++index] != '"')
            {
                key.push_back(content[index]);
            }

            //找到key的 :
            while (content[++index] != ':')
            {
                ++index;
            }

            ++index; //跳过 :

            SkipBlank(content, index);

            JsonObjectPrivate value;
            if (content[index] == '{')
            {
                value.ParseObject(content, index);
                AddJsonObj(key, value);
            }
            else if (content[index] == '[')
            {
                value = ParseArray(content, index);
                AddJsonObj(key, value);
            }
            else if (IsValueBegin(content[index]))
            {
                value = ParseValue(content, index);
                AddJsonObj(key, value);
            }
            else
            {
                std::cout << "error JsonObject" << std::endl;
            }
            key.clear();
        }
    }
    return *this;
}

my_sdk::JsonObjectPrivate my_sdk::JsonObjectPrivate::ParseArray(const std::string& content, size_t& index)
{
    JsonObjectPrivate result;
    result.SetValueType(JsonValue::EM_JsonValue::Array);
    if (content[index] == '[')
    {
        ++index;
        JsonObjectPrivate value;
        if (IsValueBegin(content[index]))
        {
            value = ParseValue(content, index);
            result.AddArrayValue(value);
        }
        else if (content[index] == '[')
        {
            value.ParseArray(content, index);
            result.AddArrayValue(value);
        }
        else if (content[index] == '{')
        {
            value.ParseObject(content, index);
            result.AddArrayValue(value);
        }
        else
        {
            result.SetValueType(JsonValue::EM_JsonValue::Error);
        }
    }
    return result;
}

my_sdk::JsonObjectPrivate my_sdk::JsonObjectPrivate::ParseValue(const std::string& content, size_t& index)
{
    int bNumber = content[index] - '0';
    JsonObjectPrivate result;
    while (index < content.size())
    {
        if (content[index] == ',' || content[index] == '}' || content[index] == ']')
        {
            break;
        }
        //string
        if (content[index] == '"')
        {
            result.SetValueType(JsonValue::EM_JsonValue::String);
            std::string str;
            JsonValue value;
            while (content[++index] != '"')
            {
                str.push_back(content[index]);
            }
            value.SetStrValue(str);
            result.SetValue(value);
            ++index;
            SkipBlank(content, index);
        }
        //number
        else if (bNumber >= 0 && bNumber <= 9)
        {
            result.SetValueType(JsonValue::EM_JsonValue::Number);
            std::string numberStr;
            bool hasDecimal = false;  // 是否包含小数点
            bool hasExponent = false; // 是否包含指数

            // 允许负号开头
            if (content[index] == '-')
            {
                numberStr += content[index++];
            }

            while (index < content.size())
            {
                const char c = content[index];

                // 检查数字组成部分
                if (isdigit(c))
                {
                    numberStr += c;
                    ++index;
                }
                // 处理小数点
                else if (c == '.' && !hasDecimal && !hasExponent)
                {
                    if (numberStr.empty() || (!isdigit(numberStr.back()) && numberStr.back() != '-'))
                    {
                        result.SetValueType(JsonValue::EM_JsonValue::Error);
                        break;
                    }
                    numberStr += c;
                    hasDecimal = true;
                    ++index;
                }
                // 处理科学计数法
                else if ((c == 'e' || c == 'E') && !hasExponent)
                {
                    if (numberStr.empty() || (!isdigit(numberStr.back()) && numberStr.back() != '.'))
                    {
                        result.SetValueType(JsonValue::EM_JsonValue::Error);
                        break;
                    }
                    numberStr += c;
                    hasExponent = true;
                    ++index;

                    // 处理指数符号
                    if (index < content.size() && (content[index] == '+' || content[index] == '-'))
                    {
                        numberStr += content[index++];
                    }
                }
                // 结束条件：遇到非数字成分
                else if (c == ',' || c == '}' || c == ']' || isspace(c))
                {
                    break;
                }
                else
                {
                    result.SetValueType(JsonValue::EM_JsonValue::Error);
                    break;
                }
            }

            // 验证最终格式
            if (numberStr.empty() ||
                numberStr == "-" ||
                numberStr.back() == '.' ||
                numberStr.find('e') == numberStr.size() - 1 ||
                numberStr.find('E') == numberStr.size() - 1)
            {
                result.SetValueType(JsonValue::EM_JsonValue::Error);
            }
            else
            {
                JsonValue value;
                if (hasDecimal || hasExponent)
                {
                    value.SetDoubleValue(strtod(numberStr.c_str(), nullptr));
                }
                else
                {
                    value.SetIntValue(strtoll(numberStr.c_str(), nullptr, 10));
                }
                result.SetValue(value);
            }
            SkipBlank(content, index);
        }
        //boolean
        else if (content[index] == 'f' || content[index] == 't')
        {
            if ((content[index] == 'f' && content.substr(index, 5) == "false") || (content[index] == 't' && content.substr(index, 4) == "true"))
            {
                result.SetValueType(JsonValue::EM_JsonValue::Boolean);
                JsonValue value;
                if (content[index] == 'f')
                {
                    value.SetStrValue("false");
                    index += 5;
                }
                else
                {
                    value.SetStrValue("true");
                    index += 4;
                }
                result.SetValue(value);
            }
            else
            {
                result.SetValueType(JsonValue::EM_JsonValue::Error);
            }
            SkipBlank(content, index);
        }
        //null
        else if (content[index] == 'n')
        {
            if (content.substr(index, 4) == "null")
            {
                JsonValue value;
                value.SetStrValue("null");
                result.SetValueType(JsonValue::EM_JsonValue::Null);
                index += 4;
            }
            else
            {
                result.SetValueType(JsonValue::EM_JsonValue::Error);
            }
            SkipBlank(content, index);
        }
        //error
        else
        {
            result.SetValueType(JsonValue::EM_JsonValue::Error);
        }
    }

    return result;
}

void my_sdk::JsonObjectPrivate::SetValueType(const JsonValue::EM_JsonValue& enumValue)
{
    m_valueType = enumValue;
}

my_sdk::JsonValue::EM_JsonValue my_sdk::JsonObjectPrivate::GetValueType() const
{
    return m_valueType;
}

void my_sdk::JsonObjectPrivate::AddJsonObj(const std::string& key, const JsonObjectPrivate& jsonObject)
{
    if (m_object.find(key) != m_object.end())
    {
        return;
    }

    m_object.insert(std::make_pair(key, jsonObject));
}

const std::unordered_map<std::string, my_sdk::JsonObjectPrivate>& my_sdk::JsonObjectPrivate::GetCurJsonObject() const
{
    return m_object;
}

void my_sdk::JsonObjectPrivate::SetCurJsonObject(const std::unordered_map<std::string, JsonObjectPrivate>& value)
{
    m_object = value;
}

void my_sdk::JsonObjectPrivate::SetValue(const JsonValue& value)
{
    m_Jsonvalue = value;
}

my_sdk::JsonValue& my_sdk::JsonObjectPrivate::GetValue()
{
    return m_Jsonvalue;
}

void my_sdk::JsonObjectPrivate::AddArrayValue(const JsonObjectPrivate& value)
{
    m_JsonValueArray.push_back(value);
}

std::vector<my_sdk::JsonObjectPrivate>& my_sdk::JsonObjectPrivate::GetJsonArrayValue()
{
    return m_JsonValueArray;
}

bool my_sdk::JsonObjectPrivate::IsValueBegin(const char& str)
{
    if (str == '"' || isdigit(str) || str == '-' || str == 't' || str == 'f' || str == 'n')
    {
        return true;
    }

    return false;
}

void my_sdk::JsonObjectPrivate::SkipBlank(const std::string& content, size_t& index)
{
    //跳过空格
    while (content[index] == ' ' || content[index] == '\n' || content[index] == '\t' || content[index] == '\r')
    {
        ++index;
    }
}