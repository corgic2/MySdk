#pragma once
#include "../SDKCommonDefine/SDKCommonDefine.h"
#include <iostream>

class SinglePatternClass
{
private:
	SinglePatternClass() = default;
	~SinglePatternClass()
	{
		SAFE_DELETE_POINTER_VALUE(m_obj)
	}
public:
	SinglePatternClass& operator=(const SinglePatternClass& obj) = delete;
	SinglePatternClass(const SinglePatternClass& obj) = delete;
	static SinglePatternClass* getinstance()
	{
		if (nullptr == m_obj)
		{
			m_obj = new SinglePatternClass;
		}
		return m_obj;
	}
	void print()
	{
		std::cout << "SinglePatternClass Print Function Used" << std::endl;
	}
private:
	static SinglePatternClass* m_obj;
};

