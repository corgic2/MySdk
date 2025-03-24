#include "JsonValue.h"

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
    }

    return *this;
}

void my_sdk::JsonValue::Clear()
{
    m_strValue = "";
}

void my_sdk::JsonValue::SetStrValue(const std::string& strValue)
{
    m_strValue = strValue;
}

std::string my_sdk::JsonValue::GetStrValue()
{
    return m_strValue;
}