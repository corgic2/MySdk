#pragma once
#include <iostream>
#include <string>
#include "JsonObject.h"
using namespace my_sdk;
void JsonObjectTest()
{
    my_sdk::JsonObject jsonObj("../TestFiles", "testForJsonObject.json");
    std::cout << "JsonObject end" << std::endl;
}
