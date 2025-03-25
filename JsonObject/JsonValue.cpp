#include "JsonValue.h"

namespace my_sdk
{
    JsonValue::JsonValue()
        : m_type(EM_JsonValue::Error),
        m_strValue(""),
        m_isNumberInt(false)
    {
        m_intValue = 0; // 初始化联合体
    }

    JsonValue::~JsonValue() = default;

    JsonValue::JsonValue(const JsonValue& obj)
        : m_type(obj.m_type),
        m_strValue(obj.m_strValue),
        m_isNumberInt(obj.m_isNumberInt)
    {
        if (m_type == EM_JsonValue::Number)
        {
            if (m_isNumberInt)
            {
                m_intValue = obj.m_intValue;
            }
            else
            {
                m_doubleValue = obj.m_doubleValue;
            }
        }
    }

    JsonValue& JsonValue::operator=(const JsonValue& obj)
    {
        if (this != &obj)
        {
            m_type = obj.m_type;
            m_strValue = obj.m_strValue;
            m_isNumberInt = obj.m_isNumberInt;
            if (m_type == EM_JsonValue::Number)
            {
                if (m_isNumberInt)
                {
                    m_intValue = obj.m_intValue;
                }
                else
                {
                    m_doubleValue = obj.m_doubleValue;
                }
            }
        }
        return *this;
    }

    void JsonValue::Clear()
    {
        m_type = EM_JsonValue::Error;
        m_strValue.clear();
        m_isNumberInt = false;
        m_intValue = 0;
    }

    //--- 设置值的方法 ---
    void JsonValue::SetValueType(EM_JsonValue type)
    {
        m_type = type;
    }

    void JsonValue::SetStrValue(const std::string& strValue)
    {
        m_strValue = strValue;
    }

    void JsonValue::SetIntValue(int64_t val)
    {
        m_type = EM_JsonValue::Number;
        m_isNumberInt = true;
        m_intValue = val;
    }

    void JsonValue::SetDoubleValue(double val)
    {
        m_type = EM_JsonValue::Number;
        m_isNumberInt = false;
        m_doubleValue = val;
    }

    //--- 获取值的方法 ---
    JsonValue::EM_JsonValue JsonValue::GetValueType() const
    {
        return m_type;
    }

    std::string JsonValue::GetStrValue() const
    {
        return m_strValue;
    }

    int64_t JsonValue::GetIntValue() const
    {
        if (m_type != EM_JsonValue::Number || !m_isNumberInt)
        {
            return 0; // 或抛出异常
        }
        return m_intValue;
    }

    double JsonValue::GetDoubleValue() const
    {
        if (m_type != EM_JsonValue::Number)
        {
            return 0.0; // 或抛出异常
        }
        return (m_isNumberInt) ? static_cast<double>(m_intValue) : m_doubleValue;
    }
}