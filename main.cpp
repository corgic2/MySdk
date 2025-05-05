#include <iostream>
#include <string>

#include "DesignPattern/SinglePattern/SinglePattern.h"
#include "FileSystem/FileSystemTest.h"
#include "JsonObject/JsonObjectTest.h"
#include "SDKCommonDefine/SDKUtils.h"
#include "IniParseObject/IniParseObjectTest.h"
#include "../LogSystem/LogSystem.h"
int main()
{
    // FileSystemTest();
    //JsonObjectTest();
    // std::string value = my_utils::MyUtils::getinstance()->AddBigInterNumber("12312453152344523452346234623452", "43252346234985798465238497");
    // std::cout << value << std::endl;
    // SinglePatternClass::getinstance()->print();
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
    return 0;
}