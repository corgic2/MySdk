#pragma once
#include <unordered_map>
#include <vector>
#include "JsonValue.h"
namespace my_sdk
{
    // Json对象内部解析
    class JsonObjectPrivate
    {
    public:
        JsonObjectPrivate();
        ~JsonObjectPrivate();

        JsonObjectPrivate(const JsonObjectPrivate& jsonObj);
        JsonObjectPrivate& operator=(const JsonObjectPrivate& jsonObj);

        // 从index开始以后的内容转化为Object,Array,Value等
        JsonObjectPrivate ParseObject(const std::string& content, size_t& index);
        JsonObjectPrivate ParseArray(const std::string& content, size_t& index);
        JsonObjectPrivate ParseValue(const std::string& content, size_t& index);

        // SetAndGet对象类型
        void SetValueType(const JsonValue::EM_JsonValue& enumValue);
        JsonValue::EM_JsonValue GetValueType() const;

        //根据当前层的Key，SetAndGet JsonObjPrivate对象
        my_sdk::JsonObjectPrivate GetJsonValue(const std::string& key) const;
        void SetJsonValue(const std::string& key, const JsonObjectPrivate& jsonObject);

        //添加Json对象
        void AddJsonObj(const std::string& key, const JsonObjectPrivate& jsonObject);

        // 获取当前存储的值
        const std::unordered_map<std::string, my_sdk::JsonObjectPrivate>& GetCurJsonObject() const;
        void SetCurJsonObject(const std::unordered_map<std::string, JsonObjectPrivate>& value);

        // 当为Value时
        void SetValue(const JsonValue& value);
        JsonValue& GetValue();

        // 当为Array时
        void AddArrayValue(const JsonObjectPrivate& value);
        std::vector<my_sdk::JsonObjectPrivate>& GetJsonArrayValue();

        bool IsValueBegin(const char& str);
        void SkipBlank(const std::string& content, size_t& index);

    private:
        JsonValue::EM_JsonValue m_valueType = JsonValue::EM_JsonValue::Object;                                              // 对JsonValue的标识
        std::unordered_map<std::string, JsonObjectPrivate> m_object = std::unordered_map<std::string, JsonObjectPrivate>(); // 当为Object时，采用这个值
        JsonValue m_Jsonvalue;                                                                                              // 当为Value时采用这个值
        std::vector<JsonObjectPrivate> m_JsonValueArray;                                                                    //当为Array时，采用这个值
    };
}