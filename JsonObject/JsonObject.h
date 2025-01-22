// ************************************************************
// <remarks>
// Author      : liaofeng
// CreateTime  : 2025-01-07
// Description : Json对象解析类
// </remarks>
// ************************************************************
#include <string>
#include <unordered_map>
#include "FileSystem.h"
#if 0
// 用于存放JsonObject对象
class JsonObjectPrivate
{
public:
    enum EM_JsonValue
    {
        Object,
        Array,
        String,
        Boolean,
        Null,
        Number
    };
    JsonObjectPrivate();
    ~JsonObjectPrivate();
    //从index开始以后的内容转化为Object,Array,Value等
    JsonObjectPrivate ParseObject(const std::string& content,size_t &index);
    JsonObjectPrivate ParseArray(const std::string& content,size_t &index);
    JsonObjectPrivate ParseValue(const std::string& content,size_t &index);

    // SetAndGet对象类型
    void SetValueType(const EM_JsonValue &enumValue);
    void GetValueType();

private:
    std::unordered_map<std::string, JsonObjectPrivate> m_JsonObject;
    EM_JsonValue m_valueType = EM_JsonValue::Object;
};

class JsonObject
{
public:
    JsonObject();
    JsonObject(const std::string &filePath); // 从文件中读取转化Json对象
    ~JsonObject();

    JsonObject &operator=(const JsonObject &obj);
    // SetAndGet 实例对象
    void SetJsonObject(const JsonObject &obj);
    JsonObjectPrivate& GetJsonObject();
private:
    std::string m_content;
    JsonObjectPrivate m_obj;
};
#endif 
