#pragma once
#include <iostream>
#include <string>
#include "JsonObject.h"
using namespace my_sdk;
void JsonObjectTest()
{
    my_sdk::JsonObject jsonObj("path/to/json", "example.json");
    my_sdk::JsonObjectPrivate& obj = jsonObj.GetJsonObject();
    // 假设JSON文件内容为：{"name": "John", "age": 30, "isStudent": false}
    std::string name = obj.GetJsonObj("name").GetValue().m_strValue;
    int age = static_cast<int>(obj.GetJsonObj("age").GetValue().m_numberValue);
    bool isStudent = obj.GetJsonObj("isStudent").GetValue().m_booleanValue;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Age: " << age << std::endl;
    std::cout << "Is Student: " << (isStudent ? "Yes" : "No") << std::endl;
}
