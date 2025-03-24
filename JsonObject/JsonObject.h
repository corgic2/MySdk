// ************************************************************
// <remarks>
// Author      : liaofeng
// CreateTime  : 2025-01-07
// Description : Json对象解析类
// </remarks>
// ************************************************************
#pragma once
#include <vector>
#include "JsonObjectPrivate.h"
#include "../FileSystem/FileSystem.h"
#if 1
namespace my_sdk
{
    class JsonObjectPrivate;

    //对外暴露的对象，提供Json操作接口
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
