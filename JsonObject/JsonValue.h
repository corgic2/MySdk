#pragma once
#include <cstdint>  // 支持 int64_t
#include <string>

namespace my_sdk
{
    class JsonValue
    {
    public:
        enum EM_JsonValue
        {
            Object,
            Array,
            String,
            Boolean,
            Null,
            Number,
            Error
        };

        JsonValue();
        ~JsonValue();

        JsonValue(const JsonValue& obj);
        JsonValue& operator=(const JsonValue& obj);
        void Clear();

        // 设置值类型和内容
        void SetValueType(EM_JsonValue type);
        void SetStrValue(const std::string& strValue);
        void SetIntValue(int64_t val);
        void SetDoubleValue(double val);

        // 获取值（需根据类型调用对应方法）
        EM_JsonValue GetValueType() const;
        std::string GetStrValue() const;
        int64_t GetIntValue() const;
        double GetDoubleValue() const;

    private:
        EM_JsonValue m_type;    // 当前存储的数据类型
        std::string m_strValue; // 存储String/Boolean/Null的字符串形式

        // 联合体存储Number类型（整数或浮点）
        union
        {
            int64_t m_intValue;
            double m_doubleValue;
        };

        bool m_isNumberInt; // 标记Number是否为整数
    };
}
