#include "JsonObject.h"

my_sdk::JsonObjectPrivate::JsonObjectPrivate()
{
}

my_sdk::JsonObjectPrivate::~JsonObjectPrivate()
{
}

my_sdk::JsonObjectPrivate::JsonObjectPrivate(const JsonObjectPrivate& jsonObj)
{
}


my_sdk::JsonValue::JsonValue()
{
}

my_sdk::JsonValue::~JsonValue()
{
}

my_sdk::JsonValue::JsonValue(const JsonValue& obj)
{
}

my_sdk::JsonValue& my_sdk::JsonValue::operator=(const JsonValue& obj)
{
    return *this;
}

void my_sdk::JsonValue::Clear()
{
}


my_sdk::JsonObjectPrivate& my_sdk::JsonObjectPrivate::operator=(const JsonObjectPrivate& jsonObj)
{
    return *this;
}

my_sdk::JsonObjectPrivate my_sdk::JsonObjectPrivate::ParseObject(const std::string& content, size_t& index)
{
    return JsonObjectPrivate();
}

my_sdk::JsonObjectPrivate my_sdk::JsonObjectPrivate::ParseArray(const std::string& content, size_t& index)
{
    return JsonObjectPrivate();
}

my_sdk::JsonObjectPrivate my_sdk::JsonObjectPrivate::ParseValue(const std::string& content, size_t& index)
{
    return JsonObjectPrivate();
}

void my_sdk::JsonObjectPrivate::SetValueType(const EM_JsonValue& enumValue)
{
}

my_sdk::EM_JsonValue my_sdk::JsonObjectPrivate::GetValueType()
{
    return m_valueType;
}

my_sdk::JsonObjectPrivate& my_sdk::JsonObjectPrivate::GetJsonObj(const std::string& key)
{
    return *this;
}

void my_sdk::JsonObjectPrivate::SetJsonObjValue(const std::string& key, const JsonObjectPrivate& jsonObject)
{
}

void my_sdk::JsonObjectPrivate::AddJsonObjToJsonArray(const JsonObjectPrivate& key)
{
}

my_sdk::JsonValue& my_sdk::JsonObjectPrivate::GetValue()
{
    return m_JsonObject;
}

void my_sdk::JsonObjectPrivate::SetValue(JsonValue& value)
{
}

my_sdk::JsonObject::JsonObject()
{
}

my_sdk::JsonObject::JsonObject(const std::string& filePath)
{
}

my_sdk::JsonObject::~JsonObject()
{
}

my_sdk::JsonObject& my_sdk::JsonObject::operator=(const JsonObject& obj)
{
    return *this;
}

void my_sdk::JsonObject::SetJsonObject(const JsonObject& obj)
{
}

my_sdk::JsonObjectPrivate& my_sdk::JsonObject::GetJsonObject()
{
    return m_obj;
}
