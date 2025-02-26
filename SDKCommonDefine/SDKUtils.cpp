#include "SDKUtils.h"

#include "SDKCommonDefine.h"

namespace my_utils
{
    MyUtils* MyUtils::m_obj = nullptr;

    MyUtils* MyUtils::getinstance()
    {
        if (m_obj == nullptr)
        {
            m_obj = new MyUtils;
        }
        return m_obj;
    }

    MyUtils::~MyUtils()
    {
        SAFE_DELETE_POINTER_VALUE(m_obj)
    }


    std::string MyUtils::AddBigInterNumber(const std::string& str1, const std::string& str2)
    {
        std::string result;
        int left = str1.size() - 1, right = str2.size() - 1;
        int cnt = 0;
        while (left >= 0 || right >= 0)
        {
            int num = 0;
            if (left >= 0)
            {
                num += str1[left--] - '0';
            }
            if (right >= 0)
            {
                num += str2[right--] - '0';
            }
            num += cnt;
            result.push_back(num % 10 + '0');
            num /= 10;
            cnt = num;
        }
        std::reverse(result.begin(), result.end());
        return result;
    }
}
