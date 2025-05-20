#include <iostream>
#include <string>

#include "DesignPattern/SinglePattern/SinglePattern.h"
#include "FileSystem/FileSystemTest.h"
#include "JsonObject/JsonObjectTest.h"
#include "SDKCommonDefine/SDKUtils.h"
#include "IniParseObject/IniParseObjectTest.h"
#include "LogSystem/LogSystem.h"
#include "ThreadPool/ThreadPool.h"
#pragma execution_character_set("utf-8")
#define LogSystemTest 0
#define THREADPOOL 1
int main()
{
    //FileSystemTest();
    //JsonObjectTest();
    //InitParseObjectTest();
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
#if THREADPOOL
    {
        ThreadPool pool; // 创建一个线程池，最多4个线程
        // 并发测试
        std::vector<std::future<int>> futures;
        auto point = std::chrono::steady_clock::now();
        for (int i = 0; i < 1e6; ++i)
        {
            // 单线程预计需要1小时完成10000000次计算
            // std::thread t([&]()
            //{
            //     return i * i;
            // });
            // t.join(); 
            futures.push_back(pool.AddTask([i]()
            {
                return i * i;
            })); // 线程池则只需2秒左右
        }
        // 验证所有结果
        for (auto &f : futures)
        {
            f.get();
        }
        auto pointNow = std::chrono::steady_clock::now();
        std::cout << "time consume : " << (pointNow - point).count() / 1e9 << " seconds" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
#endif
    std::cout << "Main Function End" << std::endl;
    return 0;
}