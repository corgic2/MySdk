#pragma once
#include <string>

struct ST_ComponentBaseInfo
{
    unsigned long cptUID;
    std::string cptName;
};

struct ST_OptearteBaseInfo
{
    unsigned long optType;
};

struct ST_NotifyBaseInfo
{
    unsigned long notifyType;
};

class AbstractFactory
{
  public:
    AbstractFactory();

    virtual ~AbstractFactory() = default; // 析构函数必须是虚函数，否则析构时不会调用派生类的析构函数

    virtual bool CreateComponent() = 0;

    virtual bool OperateComponent(ST_OptearteBaseInfo *info) = 0;

    virtual bool NotifyComponent(ST_NotifyBaseInfo *info) = 0;
};

class CptBaseObject
{
  public:
    CptBaseObject();
    virtual ~CptBaseObject();
    virtual ST_ComponentBaseInfo *CreateComponentObject() = 0;

  private:
    ST_ComponentBaseInfo *m_baseInfo = nullptr;
};