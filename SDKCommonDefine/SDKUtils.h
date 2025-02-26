#pragma once
#include <string>


namespace my_utils
{
    class MyUtils
    {
    private:
        MyUtils() = default;
        ~MyUtils();

    public:
        static MyUtils* getinstance();
        std::string AddBigInterNumber(const std::string& str1, const std::string& str2);

    private:
        static MyUtils* m_obj;
    };
}
