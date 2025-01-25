#include "JsonObject.h"
#include "../FileSystem/FileSystem.h"
#define SAFE_DELETE_POINTER_VALUE(p) {if(p) {delete p; p = nullptr;}}
#define SAFE_DELETE_POINTER_ARRAY(p) {if(p) {delete []p; p = nullptr;}}
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
    SetValueType(my_sdk::EM_JsonValue::Object);
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
        SAFE_DELETE_POINTER_VALUE(m_jsonObject);
        m_jsonObject = new JsonValue(*jsonObj.m_jsonObject);
        m_valueType = jsonObj.m_valueType;
    }

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
    m_valueType = enumValue;
}

my_sdk::EM_JsonValue my_sdk::JsonObjectPrivate::GetValueType() const
{
    return m_valueType;
}

my_sdk::JsonObjectPrivate& my_sdk::JsonObjectPrivate::GetJsonObj(const std::string& key) const
{
    if (m_valueType != my_sdk::EM_JsonValue::Object || !m_jsonObject)
    {
        return JsonObjectPrivate();
    }

    auto it = m_jsonObject->m_mapValue.find(key);

    if (it != m_jsonObject->m_mapValue.end())
    {
        return it->second;
    }
    else
    {
        return JsonObjectPrivate();
    }
}

void my_sdk::JsonObjectPrivate::SetJsonObjValue(const std::string& key, const JsonObjectPrivate& jsonObject)
{
    if (m_valueType != my_sdk::EM_JsonValue::Object || !m_jsonObject)
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
    if (m_valueType != my_sdk::EM_JsonValue::Object || !m_jsonObject)
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
    else
    {
        static JsonValue emptyObject;
        return emptyObject;
    }
}
void my_sdk::JsonObjectPrivate::SetValue(JsonValue& value)
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