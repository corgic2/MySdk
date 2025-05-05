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

    private:
        static MyUtils* m_obj;
    };
}
