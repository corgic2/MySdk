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
}
