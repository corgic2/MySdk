// ************************************************************
// <remarks>
// Author      : liaofeng
// CreateTime  : 2025-01-07
// Description : Json对象解析类
// </remarks>
// ************************************************************
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "FileSystem.h"
#if 1

namespace my_sdk
{
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

        union JsonValue{
            std::string strValue;
            bool booleanValue;
            double numberValue;
            std::nullptr_t pointerValue;
            std::vector<JsonObjectPrivate> vectorValue;
            std::unordered_map<std::string, JsonObjectPrivate> mapValue;
        };

        JsonObjectPrivate();
        ~JsonObjectPrivate();

        JsonObjectPrivate(const JsonObjectPrivate &jsonObj);
        JsonObjectPrivate &operator=(const JsonObjectPrivate &jsonObj);

        // 从index开始以后的内容转化为Object,Array,Value等
        JsonObjectPrivate ParseObject(const std::string &content, size_t &index);
        JsonObjectPrivate ParseArray(const std::string &content, size_t &index);
        JsonObjectPrivate ParseValue(const std::string &content, size_t &index);

        // SetAndGet对象类型
        void SetValueType(const EM_JsonValue &enumValue);
        EM_JsonValue GetValueType();

        //根据Key，SetAndGet JsonObjPrivate对象
        JsonObjectPrivate& GetJsonObj(const std::string& key);
        void SetJsonObjValue(const std::string& key,const JsonObjectPrivate& jsonObject);

        //给Array 添加Json对象
        void AddJsonObjToJsonArray(const JsonObjectPrivate& key);

        // 获取存储的值
        JsonValue& GetValue();
        void SetValue(JsonValue& value);
    private:
        JsonValue m_JsonObject; // 存储实际的值
        EM_JsonValue m_valueType = EM_JsonValue::Object; // 对JsonValue的标识
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
        JsonObjectPrivate &GetJsonObject();

    private:
        std::string m_content;
        JsonObjectPrivate m_obj;
    };
}
#endif
