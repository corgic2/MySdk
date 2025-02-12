// ************************************************************
// <remarks>
// Author      : liaofeng
// CreateTime  : 2025-01-07
// Description : Json对象解析类
// </remarks>
// ************************************************************
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "../FileSystem/FileSystem.h"
#if 1
namespace my_sdk
{
  class JsonObjectPrivate;

  enum EM_JsonValue
  {
    Object,
    Array,
    String,
    Boolean,
    Null,
    Number
  };

  class JsonValue
  {
  public:
    JsonValue();
    ~JsonValue();

    JsonValue(const JsonValue& obj);
    JsonValue& operator=(const JsonValue& obj);
    void Clear();

  public:
    std::string m_strValue = "";
    bool m_booleanValue = false;
    double m_numberValue = 0;
    std::nullptr_t m_pointerValue;
    std::unordered_map<std::string, JsonObjectPrivate> m_mapValue;
  };

  // 用于存放JsonObject对象
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
    void SetValueType(const EM_JsonValue& enumValue);
    EM_JsonValue GetValueType() const;

    //根据当前层的Key，SetAndGet JsonObjPrivate对象
    JsonObjectPrivate& GetJsonObj(const std::string& key) const;
    void SetJsonObjValue(const std::string& key, const JsonObjectPrivate& jsonObject);

    //添加Json对象
    void AddJsonObj(const std::string& key, const JsonObjectPrivate& jsonObject);

    // 获取存储的值
    JsonValue& GetValue() const;
    void SetValue(JsonValue& value);

  private:
    JsonValue* m_jsonObject = nullptr;               // 存储实际的值
    EM_JsonValue m_valueType = EM_JsonValue::Object; // 对JsonValue的标识
  };

  class JsonObject
  {
  public:
    JsonObject();
    JsonObject(const std::string& filePath, const std::string& fileName); // 从文件中读取转化Json对象
    ~JsonObject();

    JsonObject& operator=(const JsonObject& obj);
    // SetAndGet 实例对象
    void SetJsonObject(const JsonObject& obj);
    JsonObjectPrivate& GetJsonObject();

  private:
    std::string m_content;
    JsonObjectPrivate m_obj = JsonObjectPrivate();
  };
}
#endif
