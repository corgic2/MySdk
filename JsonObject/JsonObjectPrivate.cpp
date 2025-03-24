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
        m_object = jsonObj.GetCurJsonObject();
        m_valueType = jsonObj.m_valueType;
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

            //跳过空格
            while (content[index] == ' ' || content[index] == '\n' || content[index] == '\t')
            {
                ++index;
            }

            if (content[index] == '{')
            {
                AddJsonObj(key, ParseObject(content, index));
            }
            else if (content[index] == '[')
            {
                AddJsonObj(key, ParseArray(content, index));
            }
            else if (content[index] == '"'
                || ((content[index] - '0') >= 0 && (content[index] - '0') <= 9)
                || content[index] == 't' || content[index] == 'f' || content[index] == 'n')
            {
                AddJsonObj(key, ParseValue(content, index));
            }
            else
            {
                std::cout << "error JsonObject" << std::endl;
            }
        }
    }
    return *this;
}

my_sdk::JsonObjectPrivate my_sdk::JsonObjectPrivate::ParseArray(const std::string& content, size_t& index)
{
    if (content[index] == '[')
    {
    }
    return *this;
}

my_sdk::JsonObjectPrivate my_sdk::JsonObjectPrivate::ParseValue(const std::string& content, size_t& index)
{
    int bNumber = content[index] - '0';
    JsonObjectPrivate result;
    while (index < content.size())
    {
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
        }
        //number
        else if (bNumber >= 0 && bNumber <= 9)
        {
            result.SetValueType(JsonValue::EM_JsonValue::Number);
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
                }
                else
                {
                    value.SetStrValue("true");
                }
                result.SetValue(value);
            }
            else
            {
                result.SetValueType(JsonValue::EM_JsonValue::Error);
            }
        }
        //null
        else if (content[index] == 'n')
        {
            if (content.substr(index, 4) == "null")
            {
                JsonValue value;
                value.SetStrValue("null");
                result.SetValueType(JsonValue::EM_JsonValue::Null);
            }
            else
            {
                result.SetValueType(JsonValue::EM_JsonValue::Error);
            }
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

void my_sdk::JsonObjectPrivate::AddArrayValue(const JsonValue& value)
{
    m_JsonValueArray.push_back(value);
}

std::vector<my_sdk::JsonValue>& my_sdk::JsonObjectPrivate::GetJsonArrayValue()
{
    return m_JsonValueArray;
}