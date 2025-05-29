#pragma once
#include <iostream>
#include <string>
#include <unordered_map>

enum EM_CptType
{
    FCpt = 0,
    VCpt,
    LCpt
};
struct ST_ComponentBaseInfo
{
    unsigned long cptUID;
    std::string cptName;
    unsigned int type;
};

class CptBaseObject
{
  public:
    CptBaseObject();
    virtual ~CptBaseObject();
    void CreateComponentObject();
    virtual void RegisterComponent() = 0;
    virtual void PrintText() = 0;
public:
    std::unordered_map<unsigned long, ST_ComponentBaseInfo> m_cptMap;
};

class ComponentObject
{
public:
    ComponentObject(const ST_ComponentBaseInfo& info);
    ~ComponentObject();
};