#pragma once
#include <set>
#include <string>
#include <unordered_map>
#include "../SDKCommonDefine/SDKCommonDefine.h"

namespace my_sdk
{
    //Json单元值
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
        void InitTokenTypeMap();
        void Clear();

        void SetStrValue(const std::string& strValue);
        std::string GetStrValue();
    private:
        std::string m_strValue;
    };
}
