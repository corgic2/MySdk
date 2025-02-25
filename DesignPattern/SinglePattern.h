#pragma once

#include <iostream>

class SinglePatternClass
{
private:
	SinglePatternClass() = default;
	~SinglePatternClass() = default;
public:
	SinglePatternClass& operator=(const SinglePatternClass& obj) = delete;
	SinglePatternClass(const SinglePatternClass& obj) = delete;
	static SinglePatternClass& getinstance()
	{
		static SinglePatternClass instance;
		return instance;
	}
	void print()
	{
		std::cout << "SinglePatternClass Print Function Used" << std::endl;
	}
private:
	static SinglePatternClass* m_obj;
};

