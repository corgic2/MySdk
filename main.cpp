#include <iostream>
#include <string>

#include "DesignPattern/SinglePattern/SinglePattern.h"
#include "FileSystem/FileSystemTest.h"
#include "JsonObject/JsonObjectTest.h"
#include "SDKCommonDefine/SDKUtils.h"
#include "IniParseObject/IniParseObjectTest.h"
#include "../LogSystem/LogSystem.h"
#pragma execution_character_set("utf-8")
#define LogSystemTest 0
int main()
{
    //FileSystemTest();
    //JsonObjectTest();
    InitParseObjectTest();
    // SinglePatternClass::getinstance()->print();
#if LogSystemTest
    LogSystem::instance().set_file("D:\\Program Files\\Projects\\MySdk\\log.txt");
    auto start = std::chrono::high_resolution_clock::now();
    int id = 100;
    std::string name = "Alice";

    LOG_INFO("ID: {}, Name: {}", name, id);
    for (int i = 0; i < 10; ++i)
    {
        LOG_INFO("Frame {} rendered", i);
    }

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << "Throughput: " << 1e6 / duration.count() << " logs/ms\n";
#endif 
    return 0;
}